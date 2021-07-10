#include "DrvOculusCommon.h"
#include "OculusBackend.h"

#include "OVR_CAPI.h"
#include "../OpenOVR/stdafx.h"
#include "../OpenOVR/logging.h"
#include "../OpenOVR/libovr_wrapper.h"
#include "../OpenOVR/convert.h"

#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/Reimpl/BaseOverlay.h"
#include "../OpenOVR/Reimpl/BaseCompositor.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "../OpenOVR/Compositor/compositor.h"
#include "../OpenOVR/Misc/Config.h"
#include "../OpenOVR/Misc/ScopeGuard.h"

#include <string>

#define SESS (*ovr::session)
#define DESC (ovr::hmdDesc)

using std::string;
using EVRCompositorError = vr::IVRCompositor_022::EVRCompositorError;

void OculusBackend::SubmitFrames(
	bool showSkybox
) {
	// If the game has told OpenVR to render a skybox, and has indeed submitted a skybox, then render
	//  that instead.
	// TODO config option to disable this
	if (showSkybox && oovr_global_configuration.EnableAppRequestedCubemap() && skyboxCompositor) {
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
		WaitForTrackingData();
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

ovrTrackingState OculusBackend::GetTrackingState() {
	return trackingState;
}

void OculusBackend::SubmitSkyboxFrames() {
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
		WaitForTrackingData();
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

void OculusBackend::WaitForTrackingData() {
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
}

void OculusBackend::StoreEyeTexture(
	EVREye eye,
	const Texture_t * texture,
	const VRTextureBounds_t * bounds,
	EVRSubmitFlags submitFlags,
	bool isFirstEye
) {

	Compositor* &comp = compositors[S2O_eye(eye)];
	if (comp == nullptr) {
		size = ovr_GetFovTextureSize(SESS, ovrEye_Left, DESC.DefaultEyeFov[ovrEye_Left], 1);
		comp = BaseCompositor::CreateCompositorAPI(texture, size);
	}

	comp->LoadSubmitContext();

	auto revertToCallerContext = MakeScopeGuard([&]() {
		comp->ResetSubmitContext();
	});

	if (isFirstEye) {
		// TODO call frame-start method

		OOVR_FAILED_OVR_ABORT(ovr_GetSessionStatus(SESS, &sessionStatus));
	}

	// TODO make sure we don't start on the second eye.
	//if (sessionStatus.IsVisible) return;

	layer.Viewport[S2O_eye(eye)] = OVR::Recti(size);

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
}

IBackend::openvr_enum_t OculusBackend::SetSkyboxOverride(const vr::Texture_t * pTextures, uint32_t unTextureCount) {
	if (!oovr_global_configuration.EnableAppRequestedCubemap()) {
		return EVRCompositorError::VRCompositorError_None;
	}

	// For now, only support cubemaps, as that's what LibOVR supports
	if (unTextureCount != 6u) {
		OOVR_LOGF("Only cubemap skyboxes are supported - lat/long and stereo pair skyboxes are not supported. Supplied texture count: %d", unTextureCount);
		return EVRCompositorError::VRCompositorError_None;
	}

	// Apparently it's permissable to pass in a texture with a null handle! (The Forest does this)
	for (size_t i = 0; i < unTextureCount; i++) {
		if (pTextures[i].handle == nullptr) {
			ClearSkyboxOverride();
			return EVRCompositorError::VRCompositorError_None;
		}
	}

	// See if this is the first time we're invoked.
	if (!skyboxCompositor) {
		const auto size = ovr_GetFovTextureSize(SESS, ovrEye_Left, DESC.DefaultEyeFov[ovrEye_Left], 1);
		skyboxCompositor.reset(GetUnsafeBaseCompositor()->CreateCompositorAPI(pTextures, size));

		skyboxLayer.Orientation = OVR::Quatf::Identity();

		skyboxLayer.Header.Type = ovrLayerType_Cube;
		skyboxLayer.Header.Flags = ovrLayerFlag_HighQuality;
	}

	if (!skyboxCompositor->SupportsCubemap()) {
		// Compositor doesn't support cubemaps, nothing we can do
		return EVRCompositorError::VRCompositorError_None;
	}

	skyboxCompositor->InvokeCubemap(pTextures);
	skyboxLayer.CubeMapTexture = skyboxCompositor->GetSwapChain();

	OOVR_FAILED_OVR_ABORT(ovr_CommitTextureSwapChain(SESS, skyboxLayer.CubeMapTexture));

	// TODO submit this texture when the app is not submitting frames, or when the app has called FadeGrid

	return EVRCompositorError::VRCompositorError_None;
}

void OculusBackend::ClearSkyboxOverride(
) {
	// TODO
	//STUBBED();
}
