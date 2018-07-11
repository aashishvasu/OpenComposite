#include "stdafx.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace std;

#include "CVRCompositor022.h"

#define SESS (*ovr::session)
#define DESC (ovr::hmdDesc)

#include "GL/CAPI_GLE.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_GL.h"

// API-specific includes
#ifdef SUPPORT_GL
#include "OVR_CAPI_GL.h"
#endif
// DirectX included in the header
#ifdef SUPPORT_VK
#include "OVR_CAPI_Vk.h"
#endif

void CVRCompositor_022::SubmitFrames() {
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

CVRCompositor_022::CVRCompositor_022() {
	memset(&trackingState, 0, sizeof(ovrTrackingState));
	chains[0] = NULL;
	chains[1] = NULL;
}

CVRCompositor_022::~CVRCompositor_022() {
	// TODO
}

void CVRCompositor_022::SetTrackingSpace(ETrackingUniverseOrigin eOrigin) {
	throw "stub";
}

ETrackingUniverseOrigin CVRCompositor_022::GetTrackingSpace() {
	throw "stub";
}

EVRCompositorError CVRCompositor_022::WaitGetPoses(VR_ARRAY_COUNT(renderPoseArrayCount)TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	VR_ARRAY_COUNT(gamePoseArrayCount)TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {
	//ovr_WaitToBeginFrame(SESS, frameIndex);

	// Assume this method isn't being called between frames, b/c it really shouldn't be.
	leftEyeSubmitted = false;
	rightEyeSubmitted = false;

	double frameTiming = ovr_GetPredictedDisplayTime(SESS, frameIndex);
	trackingState = ovr_GetTrackingState(SESS, frameTiming, ovrTrue);

	return GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

EVRCompositorError CVRCompositor_022::GetLastPoses(VR_ARRAY_COUNT(renderPoseArrayCount)TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
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

EVRCompositorError CVRCompositor_022::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose,
	TrackedDevicePose_t * pOutputGamePose) {
	throw "stub";
}

EVRCompositorError CVRCompositor_022::Submit(EVREye eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {
	if (chains[0] == NULL) {
		size = ovr_GetFovTextureSize(SESS, ovrEye_Left, DESC.DefaultEyeFov[ovrEye_Left], 1);

		switch (texture->eType) {
#ifdef SUPPORT_GL
		case TextureType_OpenGL: {
			compositor = new GLCompositor(chains, size);
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
			throw string("[CVRCompositor_022::Submit] Unsupported texture type: ") + to_string(texture->eType);
		}

		for (int ieye = 0; ieye < 2; ++ieye) {
			if (!chains[ieye]) {
				throw string("Failed to create texture.");
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

	compositor->Invoke(S2O_eye(eye), texture, bounds, submitFlags);

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

void CVRCompositor_022::ClearLastSubmittedFrame() {
	throw "stub";
}

void CVRCompositor_022::PostPresentHandoff() {
	throw "stub";
}

bool CVRCompositor_022::GetFrameTiming(Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	throw "stub";
}

uint32_t CVRCompositor_022::GetFrameTimings(Compositor_FrameTiming * pTiming, uint32_t nFrames) {
	throw "stub";
}

float CVRCompositor_022::GetFrameTimeRemaining() {
	throw "stub";
}

void CVRCompositor_022::GetCumulativeStats(Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
	throw "stub";
}

void CVRCompositor_022::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground) {
	throw "stub";
}

HmdColor_t CVRCompositor_022::GetCurrentFadeColor(bool bBackground) {
	throw "stub";
}

void CVRCompositor_022::FadeGrid(float fSeconds, bool bFadeIn) {
	throw "stub";
}

float CVRCompositor_022::GetCurrentGridAlpha() {
	throw "stub";
}

EVRCompositorError CVRCompositor_022::SetSkyboxOverride(VR_ARRAY_COUNT(unTextureCount) const Texture_t * pTextures, uint32_t unTextureCount) {
	throw "stub";
}

void CVRCompositor_022::ClearSkyboxOverride() {
	throw "stub";
}

void CVRCompositor_022::CompositorBringToFront() {
	throw "stub";
}

void CVRCompositor_022::CompositorGoToBack() {
	throw "stub";
}

void CVRCompositor_022::CompositorQuit() {
	throw "stub";
}

bool CVRCompositor_022::IsFullscreen() {
	throw "stub";
}

uint32_t CVRCompositor_022::GetCurrentSceneFocusProcess() {
	throw "stub";
}

uint32_t CVRCompositor_022::GetLastFrameRenderer() {
	throw "stub";
}

bool CVRCompositor_022::CanRenderScene() {
	throw "stub";
}

void CVRCompositor_022::ShowMirrorWindow() {
	throw "stub";
}

void CVRCompositor_022::HideMirrorWindow() {
	throw "stub";
}

bool CVRCompositor_022::IsMirrorWindowVisible() {
	throw "stub";
}

void CVRCompositor_022::CompositorDumpImages() {
	throw "stub";
}

bool CVRCompositor_022::ShouldAppRenderWithLowResources() {
	throw "stub";
}

void CVRCompositor_022::ForceInterleavedReprojectionOn(bool bOverride) {
	throw "stub";
}

void CVRCompositor_022::ForceReconnectProcess() {
	throw "stub";
}

void CVRCompositor_022::SuspendRendering(bool bSuspend) {
	throw "stub";
}

EVRCompositorError CVRCompositor_022::GetMirrorTextureD3D11(EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	throw "stub";
}

void CVRCompositor_022::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	throw "stub";
}

EVRCompositorError CVRCompositor_022::GetMirrorTextureGL(EVREye eEye, glUInt_t * pglTextureId, glSharedTextureHandle_t * pglSharedTextureHandle) {
	throw "stub";
}

bool CVRCompositor_022::ReleaseSharedGLTexture(glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle) {
	throw "stub";
}

void CVRCompositor_022::LockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	throw "stub";
}

void CVRCompositor_022::UnlockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	throw "stub";
}

uint32_t CVRCompositor_022::GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	throw "stub";
}

uint32_t CVRCompositor_022::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T * pPhysicalDevice, VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	throw "stub";
}

void CVRCompositor_022::SetExplicitTimingMode(EVRCompositorTimingMode eTimingMode) {
	throw "stub";
}

EVRCompositorError CVRCompositor_022::SubmitExplicitTimingData() {
	throw "stub";
}
