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

void BaseCompositor::GetSinglePose(vr::TrackedDeviceIndex_t index, vr::TrackedDevicePose_t* pose) {
	memset(pose, 0, sizeof(TrackedDevicePose_t));

	if (index == k_unTrackedDeviceIndex_Hmd) {
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

ovr_enum_t BaseCompositor::GetLastPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	for (size_t i = 0; i < max(gamePoseArrayCount, renderPoseArrayCount); i++) {
		TrackedDevicePose_t *renderPose = i < renderPoseArrayCount ? renderPoseArray + i : NULL;
		TrackedDevicePose_t *gamePose = i < gamePoseArrayCount ? gamePoseArray + i : NULL;

		if (renderPose) {
			GetSinglePose(i, renderPose);
		}

		if (gamePose) {
			if (renderPose) {
				*gamePose = *renderPose;
			}
			else {
				GetSinglePose(i, gamePose);
			}
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

bool BaseCompositor::GetFrameTiming(OOVR_Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	//if (pTiming->m_nSize != sizeof(OOVR_Compositor_FrameTiming)) {
	//	STUBBED();
	//}

	static int framenum = 0; // TODO do this properly

	memset(pTiming, 0, sizeof(OOVR_Compositor_FrameTiming));

	pTiming->m_nSize = sizeof(OOVR_Compositor_FrameTiming); // Set to sizeof( Compositor_FrameTiming ) // TODO in methods calling this
	pTiming->m_nFrameIndex = framenum++; // TODO
	pTiming->m_nNumFramePresents = 1; // number of times this frame was presented
	pTiming->m_nNumMisPresented = 0; // number of times this frame was presented on a vsync other than it was originally predicted to
	pTiming->m_nNumDroppedFrames = 0; // number of additional times previous frame was scanned out
	pTiming->m_nReprojectionFlags = 0;

	/** Absolute time reference for comparing frames.  This aligns with the vsync that running start is relative to. */
	pTiming->m_flSystemTimeInSeconds = 0;

	/*
	/ ** These times may include work from other processes due to OS scheduling.
	* The fewer packets of work these are broken up into, the less likely this will happen.
	* GPU work can be broken up by calling Flush.  This can sometimes be useful to get the GPU started
	* processing that work earlier in the frame. * /
	pTiming.m_flPreSubmitGpuMs; // time spent rendering the scene (gpu work submitted between WaitGetPoses and second Submit)
	pTiming.m_flPostSubmitGpuMs; // additional time spent rendering by application (e.g. companion window)
	*/
	pTiming->m_flTotalRenderGpuMs = 5; // TODO // time between work submitted immediately after present (ideally vsync) until the end of compositor submitted work
	/*
	pTiming.m_flCompositorRenderGpuMs; // time spend performing distortion correction, rendering chaperone, overlays, etc.
	pTiming.m_flCompositorRenderCpuMs; // time spent on cpu submitting the above work for this frame
	pTiming.m_flCompositorIdleCpuMs; // time spent waiting for running start (application could have used this much more time)

								   / ** Miscellaneous measured intervals. * /
	pTiming.m_flClientFrameIntervalMs; // time between calls to WaitGetPoses
	pTiming.m_flPresentCallCpuMs; // time blocked on call to present (usually 0.0, but can go long)
	pTiming.m_flWaitForPresentCpuMs; // time spent spin-waiting for frame index to change (not near-zero indicates wait object failure)
	pTiming.m_flSubmitFrameMs; // time spent in IVRCompositor::Submit (not near-zero indicates driver issue)

							 / ** The following are all relative to this frame's SystemTimeInSeconds * /
	pTiming.m_flWaitGetPosesCalledMs;
	pTiming.m_flNewPosesReadyMs;
	pTiming.m_flNewFrameReadyMs; // second call to IVRCompositor::Submit
	pTiming.m_flCompositorUpdateStartMs;
	pTiming.m_flCompositorUpdateEndMs;
	pTiming.m_flCompositorRenderStartMs;
	*/

	GetSinglePose(k_unTrackedDeviceIndex_Hmd, &pTiming->m_HmdPose); // pose used by app to render this frame

	return true;
}

uint32_t BaseCompositor::GetFrameTimings(OOVR_Compositor_FrameTiming * pTiming, uint32_t nFrames) {
	STUBBED();
}

float BaseCompositor::GetFrameTimeRemaining() {
	STUBBED();
}

//void BaseCompositor::GetCumulativeStats(Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
//	STUBBED();
//}

void BaseCompositor::FadeToColor(float fSeconds, float fRed, float fGreen, float fBlue, float fAlpha, bool bBackground) {
	fadeTime = fSeconds;
	fadeColour.r = fRed;
	fadeColour.g = fGreen;
	fadeColour.b = fBlue;
	fadeColour.a = fAlpha;

	// TODO what does background do?
}

HmdColor_t BaseCompositor::GetCurrentFadeColor(bool bBackground) {
	return fadeColour;
}

void BaseCompositor::FadeGrid(float fSeconds, bool bFadeIn) {
	STUBBED();
}

float BaseCompositor::GetCurrentGridAlpha() {
	STUBBED();
}

ovr_enum_t BaseCompositor::SetSkyboxOverride(const Texture_t * pTextures, uint32_t unTextureCount) {
	// TODO!
	//STUBBED();
	return VRCompositorError_None;
}

void BaseCompositor::ClearSkyboxOverride() {
	// TODO
	//STUBBED();
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
	// TODO
	// I'm not sure what the purpose of this function is. If you know, please tell me.
	// - ZNix
	//STUBBED();
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