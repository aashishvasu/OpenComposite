#include "stdafx.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace std;

#include "BaseCompositor.h"
using namespace vr;
using namespace IVRCompositor_022;

typedef int ovr_enum_t;

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

#define STUBBED() { \
	string str = "Hit stubbed file at " __FILE__ " func "  " line " + to_string(__LINE__); \
	MessageBoxA(NULL, str.c_str(), "Stubbed func!", MB_OK); \
	throw "stub"; \
}

void BaseCompositor::SubmitFrames() {
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

	layer.Header.Type = ovrLayerType_EyeFov;
	layer.Header.Flags = compositor->GetFlags();

	for (int eye = 0; eye < 2; ++eye) {
		layer.ColorTexture[eye] = chains[eye];
		layer.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
		layer.RenderPose[eye] = EyeRenderPose[eye];
		layer.SensorSampleTime = sensorSampleTime;
	}

	ovrLayerHeader* layers = &layer.Header;
	ovrResult result = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
	// exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
	if (!OVR_SUCCESS(result))
		throw result;

	frameIndex++;
}

BaseCompositor::BaseCompositor() {
	memset(&trackingState, 0, sizeof(ovrTrackingState));
	chains[0] = NULL;
	chains[1] = NULL;
}

BaseCompositor::~BaseCompositor() {
	// TODO
}

void BaseCompositor::SetTrackingSpace(ETrackingUniverseOrigin eOrigin) {
	ovrTrackingOrigin origin = ovrTrackingOrigin_FloorLevel;
	if (eOrigin == TrackingUniverseSeated) {
		origin = ovrTrackingOrigin_EyeLevel;
	}

	ovr_SetTrackingOriginType(SESS, origin);
}

ETrackingUniverseOrigin BaseCompositor::GetTrackingSpace() {
	STUBBED();
}

ovr_enum_t BaseCompositor::WaitGetPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {
	//ovr_WaitToBeginFrame(SESS, frameIndex);

	// Assume this method isn't being called between frames, b/c it really shouldn't be.
	leftEyeSubmitted = false;
	rightEyeSubmitted = false;

	double frameTiming = ovr_GetPredictedDisplayTime(SESS, frameIndex);
	trackingState = ovr_GetTrackingState(SESS, frameTiming, ovrTrue);

	return GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

ovr_enum_t BaseCompositor::GetLastPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

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

ovr_enum_t BaseCompositor::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose,
	TrackedDevicePose_t * pOutputGamePose) {
	STUBBED();
}

ovr_enum_t BaseCompositor::Submit(EVREye eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {
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
		case TextureType_DirectX: {
			compositor = new DX11Compositor((ID3D11Texture2D*)texture->handle, size, chains);
			break;
		}
		case TextureType_DirectX12: {
			compositor = new DX12Compositor((D3D12TextureData_t*)texture->handle, size, chains);
			break;
		}
#endif
		default:
			throw string("[BaseCompositor::Submit] Unsupported texture type: ") + to_string(texture->eType);
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

	layer.Viewport[S2O_eye(eye)] = Recti(size);

	ovrTextureSwapChain tex = chains[S2O_eye(eye)];

	compositor->Invoke(S2O_eye(eye), texture, bounds, submitFlags, layer);

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

void BaseCompositor::ClearLastSubmittedFrame() {
	STUBBED();
}

void BaseCompositor::PostPresentHandoff() {
	STUBBED();
}

//bool BaseCompositor::GetFrameTiming(Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
//	STUBBED();
//}

//uint32_t BaseCompositor::GetFrameTimings(Compositor_FrameTiming * pTiming, uint32_t nFrames) {
//	STUBBED();
//}

float BaseCompositor::GetFrameTimeRemaining() {
	STUBBED();
}

//void BaseCompositor::GetCumulativeStats(Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
//	STUBBED();
//}

void BaseCompositor::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground) {
	STUBBED();
}

HmdColor_t BaseCompositor::GetCurrentFadeColor(bool bBackground) {
	STUBBED();
}

void BaseCompositor::FadeGrid(float fSeconds, bool bFadeIn) {
	STUBBED();
}

float BaseCompositor::GetCurrentGridAlpha() {
	STUBBED();
}

ovr_enum_t BaseCompositor::SetSkyboxOverride(const Texture_t * pTextures, uint32_t unTextureCount) {
	STUBBED();
}

void BaseCompositor::ClearSkyboxOverride() {
	STUBBED();
}

void BaseCompositor::CompositorBringToFront() {
	STUBBED();
}

void BaseCompositor::CompositorGoToBack() {
	STUBBED();
}

void BaseCompositor::CompositorQuit() {
	STUBBED();
}

bool BaseCompositor::IsFullscreen() {
	STUBBED();
}

uint32_t BaseCompositor::GetCurrentSceneFocusProcess() {
	STUBBED();
}

uint32_t BaseCompositor::GetLastFrameRenderer() {
	STUBBED();
}

bool BaseCompositor::CanRenderScene() {
	STUBBED();
}

void BaseCompositor::ShowMirrorWindow() {
	STUBBED();
}

void BaseCompositor::HideMirrorWindow() {
	STUBBED();
}

bool BaseCompositor::IsMirrorWindowVisible() {
	STUBBED();
}

void BaseCompositor::CompositorDumpImages() {
	STUBBED();
}

bool BaseCompositor::ShouldAppRenderWithLowResources() {
	STUBBED();
}

void BaseCompositor::ForceInterleavedReprojectionOn(bool bOverride) {
	STUBBED();
}

void BaseCompositor::ForceReconnectProcess() {
	STUBBED();
}

void BaseCompositor::SuspendRendering(bool bSuspend) {
	STUBBED();
}

ovr_enum_t BaseCompositor::GetMirrorTextureD3D11(EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	STUBBED();
}

void BaseCompositor::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	STUBBED();
}

ovr_enum_t BaseCompositor::GetMirrorTextureGL(EVREye eEye, glUInt_t * pglTextureId, glSharedTextureHandle_t * pglSharedTextureHandle) {
	STUBBED();
}

bool BaseCompositor::ReleaseSharedGLTexture(glUInt_t glTextureId, glSharedTextureHandle_t glSharedTextureHandle) {
	STUBBED();
}

void BaseCompositor::LockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	STUBBED();
}

void BaseCompositor::UnlockGLSharedTextureForAccess(glSharedTextureHandle_t glSharedTextureHandle) {
	STUBBED();
}

uint32_t BaseCompositor::GetVulkanInstanceExtensionsRequired(VR_OUT_STRING() char * pchValue, uint32_t unBufferSize) {
	STUBBED();
}

uint32_t BaseCompositor::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T * pPhysicalDevice, char * pchValue, uint32_t unBufferSize) {
	STUBBED();
}

void BaseCompositor::SetExplicitTimingMode(ovr_enum_t eTimingMode) {
	STUBBED();
}

ovr_enum_t BaseCompositor::SubmitExplicitTimingData() {
	STUBBED();
}