#include "stdafx.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace std;

#include "CVRCompositor.h"

#define SESS (*ovr::session)
#define DESC (ovr::hmdDesc)

#include "../libovr/OculusSDK/Samples/OculusRoomTiny_Advanced/Common/Win32_GLAppUtil.h"

// API-specific includes
#ifdef SUPPORT_GL
#include "OVR_CAPI_GL.h"
#endif
// DirectX included in the header
#ifdef SUPPORT_VK
#include "OVR_CAPI_Vk.h"
#endif

OVR::GLEContext CustomGLEContext;

void CVRCompositor::SubmitFrames() {
	ovrSession &session = *ovr::session;
	ovrGraphicsLuid &luid = *ovr::luid;
	ovrHmdDesc &hmdDesc = ovr::hmdDesc;

	// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
	ovrEyeRenderDesc eyeRenderDesc[2];
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	// Get eye poses, feeding in correct IPD offset
	ovrPosef EyeRenderPose[2];
	ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose,
		eyeRenderDesc[1].HmdToEyePose };

	double sensorSampleTime;    // sensorSampleTime is fed into the layer later
	ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

	//// Render Scene to Eye Buffers
	//for (int eye = 0; eye < 2; ++eye) {
	//	// Switch to eye render target
	//	GLuint curTexId;
	//	int curIndex;
	//	ovr_GetTextureSwapChainCurrentIndex(session, chains[eye], &curIndex);
	//	ovr_GetTextureSwapChainBufferGL(session, chains[eye], curIndex, &curTexId);

	//	// Commit changes to the textures so they get picked up frame
	//	ovr_CommitTextureSwapChain(session, chains[eye]);
	//}

	// Do distortion rendering, Present and flush/sync

	ovrLayerEyeFov ld;
	ld.Header.Type = ovrLayerType_EyeFov;
	ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

	for (int eye = 0; eye < 2; ++eye) {
		ld.ColorTexture[eye] = chains[eye];
		ld.Viewport[eye] = Recti(size);
		ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
		ld.RenderPose[eye] = EyeRenderPose[eye];
		ld.SensorSampleTime = sensorSampleTime;
	}

	ovrLayerHeader* layers = &ld.Header;
	ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
	// exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
	if (!OVR_SUCCESS(result))
		throw result;

	frameIndex++;
}

CVRCompositor::CVRCompositor() {
	memset(&trackingState, 0, sizeof(ovrTrackingState));
	chains[0] = NULL;
	chains[1] = NULL;
}

CVRCompositor::~CVRCompositor() {
	// TODO
}

void CVRCompositor::SetTrackingSpace(ETrackingUniverseOrigin eOrigin) {
	throw "stub";
}

ETrackingUniverseOrigin CVRCompositor::GetTrackingSpace() {
	throw "stub";
}

EVRCompositorError CVRCompositor::WaitGetPoses(VR_ARRAY_COUNT(renderPoseArrayCount)TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	VR_ARRAY_COUNT(gamePoseArrayCount)TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {
	//ovr_WaitToBeginFrame(SESS, frameIndex);

	// Assume this method isn't being called between frames, b/c it really shouldn't be.
	leftEyeSubmitted = false;
	rightEyeSubmitted = false;

	double frameTiming = ovr_GetPredictedDisplayTime(SESS, frameIndex);
	trackingState = ovr_GetTrackingState(SESS, frameTiming, ovrTrue);

	return GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

EVRCompositorError CVRCompositor::GetLastPoses(VR_ARRAY_COUNT(renderPoseArrayCount)TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	VR_ARRAY_COUNT(gamePoseArrayCount)TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	if (gamePoseArrayCount != 0)
		throw "Game poses not yet supported!";

	for (size_t i = 0; i < renderPoseArrayCount; i++) {
		TrackedDevicePose_t *pose = renderPoseArray + i;

		memset(pose, 0, sizeof(TrackedDevicePose_t));

		if (i == k_unTrackedDeviceIndex_Hmd) {
			pose->bPoseIsValid = true;

			// TODO deal with the HMD not being connected
			pose->bDeviceIsConnected = true;

			// TODO
			pose->eTrackingResult = TrackingResult_Running_OK;

			ovrPoseStatef &hmdPose = trackingState.HeadPose;

			O2S_v3f(hmdPose.LinearVelocity, pose->vVelocity);
			O2S_v3f(hmdPose.AngularVelocity, pose->vAngularVelocity);

			Posef ovrPose(hmdPose.ThePose);
			Matrix4f hmdTransform(ovrPose);

			O2S_om34(hmdTransform, pose->mDeviceToAbsoluteTracking);
		}
		else {
			pose->bPoseIsValid = false;
			pose->bDeviceIsConnected = false;
		}
	}

	return VRCompositorError_None;
}

EVRCompositorError CVRCompositor::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose,
	TrackedDevicePose_t * pOutputGamePose) {
	throw "stub";
}

EVRCompositorError CVRCompositor::Submit(EVREye eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {
	if (chains[0] == NULL) {
		size = ovr_GetFovTextureSize(SESS, ovrEye_Left, DESC.DefaultEyeFov[ovrEye_Left], 1);

		switch (texture->eType) {
#ifdef SUPPORT_GL
		case TextureType_OpenGL: {

			// TODO does this interfere with the host application?
			// TODO move somewhere more permenent
			OVR::GLEContext::SetCurrentContext(&CustomGLEContext);
			CustomGLEContext.Init();

			// Create the framebuffer we use when copying across textures
			glGenFramebuffers(1, &fboId);

			// Make eye render buffers
			for (int ieye = 0; ieye < 2; ++ieye) {
				ovrTextureSwapChainDesc desc = {};
				desc.Type = ovrTexture_2D;
				desc.ArraySize = 1;
				desc.Width = size.w;
				desc.Height = size.h;
				desc.MipLevels = 1;
				desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
				desc.SampleCount = 1;
				desc.StaticImage = ovrFalse;

				ovrResult result = ovr_CreateTextureSwapChainGL(SESS, &desc, &chains[ieye]);
				if (!OVR_SUCCESS(result))
					throw string("Cannot create GL texture swap chain");
			}
			break;
		}
#endif
#ifdef SUPPORT_DX
		case TextureType_DirectX12: {
			compositor = new DX12Compositor((D3D12TextureData_t*)texture->handle, size, chains);
			break;
		}
#endif
		default:
			throw string("[CVRCompositor::Submit] Unsupported texture type: ") + to_string(texture->eType);
		}

		for (int ieye = 0; ieye < 2; ++ieye) {
			if (!chains[ieye]) {
				VALIDATE(false, "Failed to create texture.");
			}
		}
	}

	if (!leftEyeSubmitted && !rightEyeSubmitted) {
		// TODO call frame-start method

		ovr_GetSessionStatus(SESS, &sessionStatus);
	}

	// TODO make sure we don't start on the second eye.
	//if (sessionStatus.IsVisible) return;

	ovrTextureSwapChain tex = chains[S2O_eye(eye)];
	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(SESS, tex, &currentIndex);

	// TODO other graphics APIs
	switch (texture->eType) {
#ifdef SUPPORT_GL
	case TextureType_OpenGL: {

		// Why TF does OpenVR pass GLuints as pointers?!? That's really unsafe, although it's
		// very unlikely GLuint is suddenly going to grow.
		GLuint src = (GLuint)texture->handle;

		GLuint texId;
		ovr_GetTextureSwapChainBufferGL(SESS, tex, currentIndex, &texId);

		//glClearColor(1, 0, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT);

		// Bind the source texture as a framebuffer, then copy out into our destination texture
		// https://stackoverflow.com/questions/23981016/best-method-to-copy-texture-to-texture#23994979
		// Since I cannot be convinved to bother with a passthrough shader and this is almost as
		// fast, we'll us it until supersampling becomes an issue.

		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src, 0);

		glBindTexture(GL_TEXTURE_2D, texId);

		GLsizei width, height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		glCopyTexSubImage2D(
			GL_TEXTURE_2D, 0, // 0 == no mipmapping
			0, 0, // Position in the source framebuffer
			0, 0, width, height // Region of the output texture to copy into (in this case, everything)
		);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		break;
	}
#endif
#ifdef SUPPORT_DX
	case TextureType_DirectX12: {
		compositor->Invoke(S2O_eye(eye), texture, bounds, submitFlags);
		break;
	}
#endif
	default:
		throw string("[CVRCompositor::Submit] Unsupported texture type: ") + to_string(texture->eType);
	}

	ovr_CommitTextureSwapChain(SESS, tex);

	//{
	//	int oeye = S2O_eye(eye);
	//	// Switch to eye render target
	//	GLuint curTexId;
	//	int curIndex;
	//	ovr_GetTextureSwapChainCurrentIndex(SESS, chains[oeye], &curIndex);
	//	ovr_GetTextureSwapChainBufferGL(SESS, chains[oeye], curIndex, &curTexId);

	//	// Commit changes to the textures so they get picked up frame
	//	ovr_CommitTextureSwapChain(SESS, chains[oeye]);
	//}

	bool state;
	if (eye == Eye_Left)
		state = leftEyeSubmitted;
	else
		state = rightEyeSubmitted;

	if (state) {
		throw "Eye already submitted!";
	}

	if (eye == Eye_Left)
		leftEyeSubmitted = true;
	else
		rightEyeSubmitted = true;

	if (leftEyeSubmitted && rightEyeSubmitted) {
		SubmitFrames();
		leftEyeSubmitted = false;
		rightEyeSubmitted = false;
	}

	return VRCompositorError_None;
}

void CVRCompositor::ClearLastSubmittedFrame() {
	throw "stub";
}

void CVRCompositor::PostPresentHandoff() {
	throw "stub";
}

bool CVRCompositor::GetFrameTiming(Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	throw "stub";
}

uint32_t CVRCompositor::GetFrameTimings(Compositor_FrameTiming * pTiming, uint32_t nFrames) {
	throw "stub";
}

float CVRCompositor::GetFrameTimeRemaining() {
	throw "stub";
}

void CVRCompositor::GetCumulativeStats(Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
	throw "stub";
}

void CVRCompositor::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground) {
	throw "stub";
}

HmdColor_t CVRCompositor::GetCurrentFadeColor(bool bBackground) {
	throw "stub";
}

void CVRCompositor::FadeGrid(float fSeconds, bool bFadeIn) {
	throw "stub";
}

float CVRCompositor::GetCurrentGridAlpha() {
	throw "stub";
}

EVRCompositorError CVRCompositor::SetSkyboxOverride(VR_ARRAY_COUNT(unTextureCount) const Texture_t * pTextures, uint32_t unTextureCount) {
	throw "stub";
}

void CVRCompositor::ClearSkyboxOverride() {
	throw "stub";
}

void CVRCompositor::CompositorBringToFront() {
	throw "stub";
}

void CVRCompositor::CompositorGoToBack() {
	throw "stub";
}

void CVRCompositor::CompositorQuit() {
	throw "stub";
}

bool CVRCompositor::IsFullscreen() {
	throw "stub";
}

uint32_t CVRCompositor::GetCurrentSceneFocusProcess() {
	throw "stub";
}

uint32_t CVRCompositor::GetLastFrameRenderer() {
	throw "stub";
}

bool CVRCompositor::CanRenderScene() {
	throw "stub";
}

void CVRCompositor::ShowMirrorWindow() {
	throw "stub";
}

void CVRCompositor::HideMirrorWindow() {
	throw "stub";
}

bool CVRCompositor::IsMirrorWindowVisible() {
	throw "stub";
}

void CVRCompositor::CompositorDumpImages() {
	throw "stub";
}

bool CVRCompositor::ShouldAppRenderWithLowResources() {
	throw "stub";
}

void CVRCompositor::ForceInterleavedReprojectionOn(bool bOverride) {
	throw "stub";
}

void CVRCompositor::ForceReconnectProcess() {
	throw "stub";
}

void CVRCompositor::SuspendRendering(bool bSuspend) {
	throw "stub";
}

vr::EVRCompositorError CVRCompositor::GetMirrorTextureD3D11(vr::EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	throw "stub";
}

void CVRCompositor::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	throw "stub";
}

vr::EVRCompositorError CVRCompositor::GetMirrorTextureGL(vr::EVREye eEye, vr::glUInt_t * pglTextureId, vr::glSharedTextureHandle_t * pglSharedTextureHandle) {
	throw "stub";
}

bool CVRCompositor::ReleaseSharedGLTexture(vr::glUInt_t glTextureId, vr::glSharedTextureHandle_t glSharedTextureHandle) {
	throw "stub";
}

void CVRCompositor::LockGLSharedTextureForAccess(vr::glSharedTextureHandle_t glSharedTextureHandle) {
	throw "stub";
}

void CVRCompositor::UnlockGLSharedTextureForAccess(vr::glSharedTextureHandle_t glSharedTextureHandle) {
	throw "stub";
}

uint32_t CVRCompositor::GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	throw "stub";
}

uint32_t CVRCompositor::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T * pPhysicalDevice, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	throw "stub";
}

void CVRCompositor::SetExplicitTimingMode(bool bExplicitTimingMode) {
	throw "stub";
}

EVRCompositorError CVRCompositor::SubmitExplicitTimingData() {
	throw "stub";
}
