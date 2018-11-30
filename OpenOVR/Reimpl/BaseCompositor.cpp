#include "stdafx.h"
#define BASE_IMPL

#include "Misc/Config.h"

#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "convert.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace std;

#include "BaseCompositor.h"
#include "BaseOverlay.h"

// For the left and right hand constants - TODO move them to their own file
#include "BaseSystem.h"
#include "static_bases.gen.h"

// Need the LibOVR Vulkan headers for the GetVulkan[Device|Instance]ExtensionsRequired methods
#if defined(SUPPORT_VK)
#include "OVR_CAPI_Vk.h"
#endif

#include "Misc/ScopeGuard.h"

using namespace vr;
using namespace IVRCompositor_022;

typedef int ovr_enum_t;

#define SESS (*ovr::session)
#define DESC (ovr::hmdDesc)

void BaseCompositor::SubmitFrames() {
	// If the game has told OpenVR to render a skybox, and has indeed submitted a skybox, then render
	//  that instead.
	// TODO config option to disable this
	if (isInSkybox && oovr_global_configuration.EnableAppRequestedCubemap() && skyboxCompositor) {
		SubmitSkyboxFrames();
		return;
	}

	if (state == RS_RENDERING || !oovr_global_configuration.ThreePartSubmit()) {
		// We're in the correct state to submit frames
	}
	else if (state == RS_NOT_STARTED) {
		// This is our first frame, skip it as the swap chains won't have been created yet
		// However, the swap chains should be available now, ready for the next frame.
		state = RS_WAIT_BEGIN;
		return;
	}
	else if (state == RS_WAIT_BEGIN) {
		OOVR_LOG("[WARN] Should not submit frames twice in a row without waiting");
		WaitGetPoses(NULL, 0, NULL, 0);
	}

	ovrSession &session = *ovr::session;
	ovrHmdDesc &hmdDesc = ovr::hmdDesc;

	// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
	ovrEyeRenderDesc *eyeRenderDesc = ovr::eyeRenderDesc;
	eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

	// Get eye poses, feeding in correct IPD offset
	ovrPosef EyeRenderPose[2];
	ovrPosef *HmdToEyePose = ovr::hmdToEyeViewPose;
	HmdToEyePose[0] = eyeRenderDesc[0].HmdToEyePose;
	HmdToEyePose[1] = eyeRenderDesc[1].HmdToEyePose;

	ovr_CalcEyePoses2(trackingState.HeadPose.ThePose, HmdToEyePose, EyeRenderPose);

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
	layer.Header.Flags = compositors[0]->GetFlags();

	for (int eye = 0; eye < 2; ++eye) {
		layer.ColorTexture[eye] = compositors[eye]->GetSwapChain();
		layer.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
		layer.RenderPose[eye] = EyeRenderPose[eye];
		layer.SensorSampleTime = sensorSampleTime;
	}

	// If the overlay system is currently active, ask it for the list of layers
	//  we should send to LibOVR. That way, it can add in layers from overlays,
	//  the virtual keyboard, etc.
	int layer_count;
	ovrLayerHeader const* const* layers;
	ovrLayerHeader* app_layer = &layer.Header;

	BaseOverlay *overlay = GetUnsafeBaseOverlay();
	if (overlay) {
		// Let the overlay system add in it's layers
		layer_count = overlay->_BuildLayers(app_layer, layers);
	}
	else {
		// Use the single layer, since the overlay system isn't in use
		layer_count = 1;
		layers = &app_layer;
	}

	// Submit the layers
	if (oovr_global_configuration.ThreePartSubmit()) {
		OOVR_FAILED_OVR_ABORT(ovr_EndFrame(session, frameIndex, nullptr, layers, layer_count));
	}
	else {
		OOVR_FAILED_OVR_ABORT(ovr_SubmitFrame(session, frameIndex, nullptr, layers, layer_count));
	}

	state = RS_WAIT_BEGIN;

	frameIndex++;

	BaseSystem *sys = GetUnsafeBaseSystem();
	if (sys) {
		sys->_OnPostFrame();
	}
}

void BaseCompositor::SubmitSkyboxFrames() {
	if (state == RS_RENDERING || !oovr_global_configuration.ThreePartSubmit()) {
		// We're in the correct state to submit frames
	}
	else if (state == RS_NOT_STARTED) {
		// This is our first frame, skip it as the swap chains won't have been created yet
		// However, the swap chains should be available now, ready for the next frame.
		state = RS_WAIT_BEGIN;
		return;
	}
	else if (state == RS_WAIT_BEGIN) {
		// Not a problem for skyboxes - this may be called many frames in a row, and that's fine
		WaitGetPoses(NULL, 0, NULL, 0);
	}

	ovrSession &session = *ovr::session;

	// If the overlay system is currently active, ask it for the list of layers
	//  we should send to LibOVR. That way, it can add in layers from overlays,
	//  the virtual keyboard, etc.
	//
	// Note we need to do this even when waiting, as it seems some apps add overlays with progress bars, etc.
	// TODO confirm this matches SteamVR behaviour
	int layer_count;
	ovrLayerHeader const* const* layers;
	ovrLayerHeader* app_layer = &skyboxLayer.Header;

	BaseOverlay *overlay = GetUnsafeBaseOverlay();
	if (overlay) {
		// Let the overlay system add in it's layers
		layer_count = overlay->_BuildLayers(app_layer, layers);
	}
	else {
		// Use the single layer, since the overlay system isn't in use
		layer_count = 1;
		layers = &app_layer;
	}

	if (oovr_global_configuration.ThreePartSubmit()) {
		OOVR_FAILED_OVR_ABORT(ovr_EndFrame(session, frameIndex, nullptr, layers, layer_count));
	}
	else {
		OOVR_FAILED_OVR_ABORT(ovr_SubmitFrame(session, frameIndex, nullptr, layers, layer_count));
	}

	state = RS_WAIT_BEGIN;

	frameIndex++;

	// Don't call BaseSystem::_OnPostFrame here, as we don't want to get input events while waiting
}

BaseCompositor::BaseCompositor() {
	memset(&trackingState, 0, sizeof(ovrTrackingState));
}

BaseCompositor::~BaseCompositor() {
	for (int eye = 0; eye < 2; eye++) {
		if (compositors[eye])
			delete compositors[eye];
	}
}

void BaseCompositor::SetTrackingSpace(ETrackingUniverseOrigin eOrigin) {
	ovrTrackingOrigin origin = ovrTrackingOrigin_FloorLevel;
	if (eOrigin == TrackingUniverseSeated) {
		origin = ovrTrackingOrigin_EyeLevel;
	}

	OOVR_FAILED_OVR_ABORT(ovr_SetTrackingOriginType(SESS, origin));
}

ETrackingUniverseOrigin BaseCompositor::GetTrackingSpace() {
	if (ovr_GetTrackingOriginType(SESS) == ovrTrackingOrigin_EyeLevel) {
		return TrackingUniverseSeated;
	}
	else {
		return TrackingUniverseStanding;
	}
}

ovr_enum_t BaseCompositor::WaitGetPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	// Assume this method isn't being called between frames, b/c it really shouldn't be.
	leftEyeSubmitted = false;
	rightEyeSubmitted = false;

	if (!oovr_global_configuration.ThreePartSubmit()) {
		// Do nothing at this stage
	}
	else if (state == RS_WAIT_BEGIN) {
		OOVR_FAILED_OVR_ABORT(ovr_WaitToBeginFrame(SESS, frameIndex));

		OOVR_FAILED_OVR_ABORT(ovr_BeginFrame(SESS, frameIndex));

		state = RS_RENDERING;
	}
	else if (state == RS_NOT_STARTED) {
		// Wait it out, need the swap chains to be created otherwise WaitToBeginFrame will error
	}
	else if (state == RS_RENDERING) {
		// Apparently this is indeed valid
		OOVR_LOG("[WARN] Should not call WaitGetPoses twice in a row - skipping second beginframe!");
	}

	sensorSampleTime = ovr_GetPredictedDisplayTime(SESS, frameIndex);
	trackingState = ovr_GetTrackingState(SESS, sensorSampleTime, ovrTrue);

	return GetLastPoses(renderPoseArray, renderPoseArrayCount, gamePoseArray, gamePoseArrayCount);
}

void BaseCompositor::GetSinglePose(vr::TrackedDeviceIndex_t index, vr::TrackedDevicePose_t* pose,
	ovrTrackingState &state) {
	memset(pose, 0, sizeof(TrackedDevicePose_t));

	ovrPoseStatef ovrPose;

	if (index == k_unTrackedDeviceIndex_Hmd) {
		ovrPose = state.HeadPose;
	}
	else if (index == BaseSystem::leftHandIndex || index == BaseSystem::rightHandIndex) {
		ovrPose = state.HandPoses[index == BaseSystem::leftHandIndex ? ovrHand_Left : ovrHand_Right];
	}
	else if (index == BaseSystem::thirdTouchIndex) {
		ovrTrackedDeviceType type = ovrTrackedDevice_Object0;
		ovr_GetDevicePoses(*ovr::session, &type, 1, 0, &ovrPose);
	}
	else {
		pose->bPoseIsValid = false;
		pose->bDeviceIsConnected = false;
		return;
	}

	// If we haven't yet got a frame, mark the controller as having
	// an invalid pose to avoid errors from unnormalised 0,0,0,0 quaternions
	if (!ovrPose.TimeInSeconds) {
		pose->bPoseIsValid = false;
		return;
	}

	if (index == BaseSystem::leftHandIndex || index == BaseSystem::rightHandIndex) {
		static Posef transform = Posef(Quatf(GetHandTransform()), GetHandTransform().GetTranslation());

		ovrPose.ThePose = Posef(ovrPose.ThePose) * transform;
	}

	// AFAIK we don't need to do anything like the above for the third Touch controller, since it
	//  isn't used as a controller anyway but rather a tracking device.

	// Configure the pose

	pose->bPoseIsValid = true;

	// TODO deal with the HMD not being connected
	pose->bDeviceIsConnected = true;

	// TODO
	pose->eTrackingResult = TrackingResult_Running_OK;

	O2S_v3f(ovrPose.LinearVelocity, pose->vVelocity);
	O2S_v3f(ovrPose.AngularVelocity, pose->vAngularVelocity);

	Posef thePose(ovrPose.ThePose);
	Matrix4f hmdTransform(thePose);

	O2S_om34(hmdTransform, pose->mDeviceToAbsoluteTracking);
}

void BaseCompositor::GetSinglePoseRendering(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose) {
	GetSinglePose(unDeviceIndex, pOutputPose, trackingState);
}

Matrix4f BaseCompositor::GetHandTransform() {
	float deg_to_rad = math_pi / 180;

	// The angle offset between the Touch and Vive controllers.
	// If this is incorrect, virtual hands will feel off.
	float controller_offset_angle = 39.5;

	// When testing to try and find the correct value above, uncomment
	//  this to lock the controller perfectly flat.
	// ovrPose.ThePose.Orientation = { 0,0,0,1 };

	Vector3f rotateAxis = Vector3f(1, 0, 0);
	Quatf rotation = Quatf(rotateAxis, controller_offset_angle * deg_to_rad); //count++ * 0.01f);

	Matrix4f transform(rotation);

	// Controller offset
	// Note this is about right, found by playing around in Unity until everything
	//  roughly lines up. If you want to contribute better numbers, please go ahead!
	transform.SetTranslation(Vector3f(0.0f, 0.0353f, -0.0451f));

	return transform;
}

ovr_enum_t BaseCompositor::GetLastPoses(TrackedDevicePose_t * renderPoseArray, uint32_t renderPoseArrayCount,
	TrackedDevicePose_t * gamePoseArray, uint32_t gamePoseArrayCount) {

	for (uint32_t i = 0; i < max(gamePoseArrayCount, renderPoseArrayCount); i++) {
		TrackedDevicePose_t *renderPose = i < renderPoseArrayCount ? renderPoseArray + i : NULL;
		TrackedDevicePose_t *gamePose = i < gamePoseArrayCount ? gamePoseArray + i : NULL;

		if (renderPose) {
			GetSinglePose(i, renderPose, trackingState);
		}

		if (gamePose) {
			if (renderPose) {
				*gamePose = *renderPose;
			}
			else {
				GetSinglePose(i, gamePose, trackingState);
			}
		}
	}

	return VRCompositorError_None;
}

ovr_enum_t BaseCompositor::GetLastPoseForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex, TrackedDevicePose_t * pOutputPose,
	TrackedDevicePose_t * pOutputGamePose) {

	if (unDeviceIndex < 0 || unDeviceIndex >= k_unMaxTrackedDeviceCount) {
		return VRCompositorError_IndexOutOfRange;
	}

	TrackedDevicePose_t pose;
	GetSinglePose(unDeviceIndex, &pose, trackingState);

	if (pOutputPose) {
		*pOutputPose = pose;
	}

	if (pOutputGamePose) {
		*pOutputGamePose = pose;
	}

	return VRCompositorError_None;
}

Compositor* BaseCompositor::CreateCompositorAPI(const vr::Texture_t* texture, const OVR::Sizei& fovTextureSize)
{
	Compositor* comp = nullptr;

	switch (texture->eType) {
#ifdef SUPPORT_GL
	case TextureType_OpenGL: {
		comp = new GLCompositor(fovTextureSize);
		break;
	}
#endif
#ifdef SUPPORT_DX
	case TextureType_DirectX: {
		if (!oovr_global_configuration.DX10Mode())
			comp = new DX11Compositor((ID3D11Texture2D*)texture->handle);
		else
			comp = new DX10Compositor((ID3D10Texture2D*)texture->handle);

		break;
	}
#endif
#if defined(SUPPORT_VK)
	case TextureType_Vulkan: {
		comp = new VkCompositor(texture);
		break;
	}
#endif
#if defined(SUPPORT_DX12)
	case TextureType_DirectX12: {
		compositor = new DX12Compositor((D3D12TextureData_t*)texture->handle, fovTextureSize, chains);
		break;
	}
#endif
	default:
		string err = "[BaseCompositor::Submit] Unsupported texture type: " + to_string(texture->eType);
		OOVR_ABORT(err.c_str());
	}

	if (comp->GetSwapChain() == NULL && texture->eType != TextureType_DirectX && texture->eType != TextureType_Vulkan) {
		OOVR_ABORT("Failed to create texture.");
	}

	return comp;
}

ovr_enum_t BaseCompositor::Submit(EVREye eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {
	Compositor* &comp = compositors[S2O_eye(eye)];
	if (comp == nullptr) {
		size = ovr_GetFovTextureSize(SESS, ovrEye_Left, DESC.DefaultEyeFov[ovrEye_Left], 1);
		comp = CreateCompositorAPI(texture, size);
	}

	comp->LoadSubmitContext();

	auto revertToCallerContext = MakeScopeGuard([&]() {
		comp->ResetSubmitContext();
	});

	if (!leftEyeSubmitted && !rightEyeSubmitted) {
		// TODO call frame-start method

		OOVR_FAILED_OVR_ABORT(ovr_GetSessionStatus(SESS, &sessionStatus));
	}

	// TODO make sure we don't start on the second eye.
	//if (sessionStatus.IsVisible) return;

	layer.Viewport[S2O_eye(eye)] = Recti(size);

	try {
		comp->Invoke(S2O_eye(eye), texture, bounds, submitFlags, layer);
	}
	catch (const string& ex) {
		string err = "Comp exception: " + ex;
		OOVR_ABORT(err.c_str());
	}
	catch (const char* ex) {
		string err = "Comp.C exception: " + string(ex);
		OOVR_ABORT(err.c_str());
	}
	catch (...) {
		OOVR_ABORT("Unknown compositor exception (catch r1)");
	}

	if (!comp->GetSwapChain()) {
		OOVR_ABORT("Missing swapchain");
	}

	OOVR_FAILED_OVR_ABORT(ovr_CommitTextureSwapChain(SESS, comp->GetSwapChain()));

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

	bool eyeState = false;
	if (eye == Eye_Left)
		eyeState = leftEyeSubmitted;
	else
		eyeState = rightEyeSubmitted;

	if (eyeState) {
		OOVR_ABORT("Eye already submitted!");
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
	// It appears (from the documentation) that SteamVR will, even after all frames are submitted, not begin
	//  compositing the submitted textures until WaitGetPoses is called. Thus is you want to do some rendering
	//  or game logic or whatever, it will delay the compositor. Calling this tells SteamVR that no further changes
	//  are to be made to the frame, and it can begin the compositor - in the aforementioned cases, this would be
	//  called directly after the last Submit call.
	//
	// On the other hand, LibOVR starts compositing as soon as ovr_EndFrame is called. So we don't have to do
	//  anything here.
	//
	// TODO: use ovr_EndFrame and co instead of ovr_SubmitFrame for better performance, not just here but in all cases
	//  that way we can call ovr_WaitToBeginFrame in WaitGetPoses to mimick SteamVR.
}

bool BaseCompositor::GetFrameTiming(OOVR_Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) {
	//if (pTiming->m_nSize != sizeof(OOVR_Compositor_FrameTiming)) {
	//	STUBBED();
	//}

	// TODO implement unFramesAgo

	ovrPerfStats stats;
	OOVR_FAILED_OVR_ABORT(ovr_GetPerfStats(*ovr::session, &stats));
	const ovrPerfStatsPerCompositorFrame &frame = stats.FrameStats[0];

	memset(pTiming, 0, sizeof(OOVR_Compositor_FrameTiming));

	pTiming->m_nSize = sizeof(OOVR_Compositor_FrameTiming); // Set to sizeof( Compositor_FrameTiming ) // TODO in methods calling this
	pTiming->m_nFrameIndex = frame.AppFrameIndex; // TODO is this per submitted frame or per HMD frame?
	pTiming->m_nNumFramePresents = 1; // TODO
	pTiming->m_nNumMisPresented = 0; // TODO
	pTiming->m_nNumDroppedFrames = 0; // TODO
	pTiming->m_nReprojectionFlags = 0;

	/** Absolute time reference for comparing frames.  This aligns with the vsync that running start is relative to. */
	// Note: OVR's method has no guarantees about aligning to vsync
	pTiming->m_flSystemTimeInSeconds = ovr_GetTimeInSeconds();

	/** These times may include work from other processes due to OS scheduling.
	* The fewer packets of work these are broken up into, the less likely this will happen.
	* GPU work can be broken up by calling Flush.  This can sometimes be useful to get the GPU started
	* processing that work earlier in the frame. */

	// time spent rendering the scene (gpu work submitted between WaitGetPoses and second Submit)
	// TODO this should be easy to time, using ovr_GetTimeInSeconds and storing
	//  that when WaitGetPoses (or PostPresentHandoff) and Submit are called, and calculating the difference
	pTiming->m_flPreSubmitGpuMs = 0;

	// additional time spent rendering by application (e.g. companion window)
	// AFAIK this is similar to m_flPreSubmitGpuMs, it's the time between PostPresentHandoff and WaitGetPoses
	// Probably not as important though
	pTiming->m_flPostSubmitGpuMs = 0;

	// time between work submitted immediately after present (ideally vsync) until the end of compositor submitted work
	// TODO CompositorCpuStartToGpuEndElapsedTime might be -1 if it's unavailable, handle that
	pTiming->m_flTotalRenderGpuMs = (frame.AppGpuElapsedTime + frame.CompositorCpuStartToGpuEndElapsedTime) / 1000;

	// time spend performing distortion correction, rendering chaperone, overlays, etc.
	pTiming->m_flCompositorRenderGpuMs = frame.CompositorGpuElapsedTime / 1000;

	// time spent on cpu submitting the above work for this frame
	// FIXME afaik CompositorCpuElapsedTime includes a bunch of other stuff too
	pTiming->m_flCompositorRenderCpuMs = frame.CompositorCpuElapsedTime / 1000;

	// time spent waiting for running start (application could have used this much more time)
	// TODO but probably not too important. I would imagine this is used primaraly for debugging.
	pTiming->m_flCompositorIdleCpuMs = 0;

	/** Miscellaneous measured intervals. */

	// time between calls to WaitGetPoses
	// TODO this should be easy to time ourselves using ovr_GetTimeInSeconds
	pTiming->m_flClientFrameIntervalMs = 0;

	// time blocked on call to present (usually 0.0, but can go long)
	// AFAIK LibOVR doesn't give us this information, but it's probably unimportant
	pTiming->m_flPresentCallCpuMs = 0;

	// time spent spin-waiting for frame index to change (not near-zero indicates wait object failure)
	// AFAIK LibOVR doesn't give us this information, though it sounds like this is again a debugging aid.
	pTiming->m_flWaitForPresentCpuMs;

	// time spent in IVRCompositor::Submit (not near-zero indicates driver issue)
	// We *could* time this, but I think it's unlikely there's any need to.
	//  This also depends on splitting up our SubmitFrame call into the three different calls and
	//  getting the wait into WaitGetPoses
	pTiming->m_flSubmitFrameMs;

	/** The following are all relative to this frame's SystemTimeInSeconds */
	// TODO these should be trivial to implement, just call ovr_GetTimeInSeconds at the right time
	/*
	pTiming->m_flWaitGetPosesCalledMs;
	pTiming->m_flNewPosesReadyMs;
	pTiming->m_flNewFrameReadyMs;
	pTiming->m_flCompositorUpdateStartMs;
	pTiming->m_flCompositorUpdateEndMs;
	pTiming->m_flCompositorRenderStartMs;
	*/

	// pose used by app to render this frame
	GetSinglePose(k_unTrackedDeviceIndex_Hmd, &pTiming->m_HmdPose, trackingState);

	return true;
}

uint32_t BaseCompositor::GetFrameTimings(OOVR_Compositor_FrameTiming * pTiming, uint32_t nFrames) {
	STUBBED();
}

float BaseCompositor::GetFrameTimeRemaining() {
	STUBBED();
}

void BaseCompositor::GetCumulativeStats(OOVR_Compositor_CumulativeStats * pStats, uint32_t nStatsSizeInBytes) {
	STUBBED();
}

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
	// This is the app telling SteamVR to fade from the rendered scene into the skybox, eg before the
	//  app loads a new level (this is how the default SteamVR Unity plugin works).
	//
	// Let's not bother implementing the fade (that would be a LOT of work), and just skip straight over.
	isInSkybox = bFadeIn;

	// TODO suppress input while in this mode
}

float BaseCompositor::GetCurrentGridAlpha() {
	STUBBED();
}

ovr_enum_t BaseCompositor::SetSkyboxOverride(const Texture_t * pTextures, uint32_t unTextureCount) {
	if (!oovr_global_configuration.EnableAppRequestedCubemap()) {
		return VRCompositorError_None;
	}

	// For now, only support cubemaps, as that's what LibOVR supports
	if (unTextureCount != 6u) {
		OOVR_LOGF("Only cubemap skyboxes are supported - lat/long and stereo pair skyboxes are not supported. Supplied texture count: %d", unTextureCount);
		return VRCompositorError_None;
	}

	// Apparently it's permissable to pass in a texture with a null handle! (The Forest does this)
	for (size_t i = 0; i < unTextureCount; i++) {
		if (pTextures[i].handle == nullptr) {
			ClearSkyboxOverride();
			return VRCompositorError_None;
		}
	}

	// See if this is the first time we're invoked.
	if (!skyboxCompositor) {
		const auto size = ovr_GetFovTextureSize(SESS, ovrEye_Left, DESC.DefaultEyeFov[ovrEye_Left], 1);
		skyboxCompositor.reset(GetUnsafeBaseCompositor()->CreateCompositorAPI(pTextures, size));

		skyboxLayer.Orientation = Quatf::Identity();

		skyboxLayer.Header.Type = ovrLayerType_Cube;
		skyboxLayer.Header.Flags = ovrLayerFlag_HighQuality;
	}

	if (!skyboxCompositor->SupportsCubemap()) {
		// Compositor doesn't support cubemaps, nothing we can do
		return VRCompositorError_None;
	}

	skyboxCompositor->InvokeCubemap(pTextures);
	skyboxLayer.CubeMapTexture = skyboxCompositor->GetSwapChain();

	OOVR_FAILED_OVR_ABORT(ovr_CommitTextureSwapChain(SESS, skyboxLayer.CubeMapTexture));

	// TODO submit this texture when the app is not submitting frames, or when the app has called FadeGrid

	return VRCompositorError_None;
}

void BaseCompositor::ClearSkyboxOverride() {
	// TODO
	//STUBBED();
}

void BaseCompositor::CompositorBringToFront() {
	// No actions required, Oculus runs via direct mode
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
	return true; // TODO implement
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
	// TODO put in config file
	return false;
}

void BaseCompositor::ForceInterleavedReprojectionOn(bool bOverride) {
	// Force timewarp on? Yeah right.
}

void BaseCompositor::ForceReconnectProcess() {
	// We should always be connected
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
#if defined(SUPPORT_VK)
	// Whaddya know, the Oculus and Valve methods work almost identically...
	OOVR_FAILED_OVR_ABORT(ovr_GetInstanceExtensionsVk(*ovr::luid, pchValue, &unBufferSize));
	return unBufferSize;
#else
	STUBBED();
#endif
}

uint32_t BaseCompositor::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T * pPhysicalDevice, char * pchValue, uint32_t unBufferSize) {
#if defined(SUPPORT_VK)
	// Use the default LUID, even if another physical device is passed in. TODO.
	OOVR_FAILED_OVR_ABORT(ovr_GetDeviceExtensionsVk(*ovr::luid, pchValue, &unBufferSize));
	return unBufferSize;
#else
	STUBBED();
#endif
}

void BaseCompositor::SetExplicitTimingMode(ovr_enum_t eTimingMode) {
	STUBBED();
}

ovr_enum_t BaseCompositor::SubmitExplicitTimingData() {
	STUBBED();
}