//
// Created by ZNix on 25/10/2020.
//

#include "XrBackend.h"

XrBackend::~XrBackend()
{
	DrvOpenXR::FullShutdown();
}

IHMD* XrBackend::GetPrimaryHMD()
{
	return hmd.get();
}

ITrackedDevice* XrBackend::GetDevice(
    vr::TrackedDeviceIndex_t index)
{
	switch (index) {
	case vr::k_unTrackedDeviceIndex_Hmd:
		return GetPrimaryHMD();
	default:
		return nullptr;
	}
}

void XrBackend::GetDeviceToAbsoluteTrackingPose(
    vr::ETrackingUniverseOrigin toOrigin,
    float predictedSecondsToPhotonsFromNow,
    vr::TrackedDevicePose_t* poseArray,
    uint32_t poseArrayCount)
{
	STUBBED();
}

/* Submitting Frames */
void XrBackend::CheckOrInitCompositors(const vr::Texture_t* tex)
{
	// Check we're using the session with the application's device
	if (!usingApplicationGraphicsAPI) {
		usingApplicationGraphicsAPI = true;

		OOVR_FALSE_ABORT(tex->eType == vr::TextureType_DirectX);
		auto* d3dTex = (ID3D11Texture2D*)tex->handle;
		ID3D11Device* dev = nullptr;
		d3dTex->GetDevice(&dev);

		XrGraphicsBindingD3D11KHR d3dInfo{};
		d3dInfo.type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
		d3dInfo.device = dev;
		DrvOpenXR::SetupSession(&d3dInfo);

		dev->Release();
	}

	for (std::unique_ptr<Compositor>& compositor : compositors) {
		// Skip a compositor if it's already set up
		if (compositor)
			continue;

		compositor.reset(BaseCompositor::CreateCompositorAPI(tex));
	}
}

void XrBackend::WaitForTrackingData()
{
	XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState state{ XR_TYPE_FRAME_STATE };

	// TODO start session

	OOVR_FAILED_XR_ABORT(xrWaitFrame(xr_session, &waitInfo, &state));
	xr_gbl->nextPredictedFrameTime = state.predictedDisplayTime;

	// FIXME loop until this returns true?
	// OOVR_FALSE_ABORT(state.shouldRender);

	XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	OOVR_FAILED_XR_ABORT(xrBeginFrame(xr_session, &beginInfo));

	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = xr_gbl->nextPredictedFrameTime;
	locateInfo.space = xr_gbl->floorSpace; // Should make no difference to the FOV
	XrViewState viewState = { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	XrView views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };
	OOVR_FAILED_XR_ABORT(xrLocateViews(xr_session, &locateInfo, &viewState, XruEyeCount, &viewCount, views));

	for (int eye = 0; eye < XruEyeCount; eye++) {
		projectionViews[eye].fov = views[eye].fov;
		projectionViews[eye].pose = views[eye].pose;
	}

	renderingFrame = true;
}

void XrBackend::StoreEyeTexture(
    vr::EVREye eye,
    const vr::Texture_t* texture,
    const vr::VRTextureBounds_t* bounds,
    vr::EVRSubmitFlags submitFlags,
    bool isFirstEye)
{
	CheckOrInitCompositors(texture);

	XrCompositionLayerProjectionView& layer = projectionViews[eye];
	layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;

	std::unique_ptr<Compositor>& compPtr = compositors[eye];
	OOVR_FALSE_ABORT(compPtr.get() != nullptr);
	Compositor& comp = *compPtr;

	comp.Invoke((XruEye)eye, texture, bounds, submitFlags, layer);

	// TODO store view somewhere and use it for submitting our frame
}

void XrBackend::SubmitFrames(bool showSkybox)
{
	if (!renderingFrame)
		return;
	renderingFrame = false;

	XrFrameEndInfo info{ XR_TYPE_FRAME_END_INFO };
	info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	info.displayTime = xr_gbl->nextPredictedFrameTime;

	// Pass in a projection layer, otherwise nothing will show up on the screen
	XrCompositionLayerProjection mainLayer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	mainLayer.space = xr_gbl->floorSpace; // Shouldn't matter what space we use, so long it matches views->pose
	mainLayer.views = projectionViews;
	mainLayer.viewCount = 2;

	XrCompositionLayerBaseHeader* headers[] = {
		(XrCompositionLayerBaseHeader*)&mainLayer,
	};
	info.layers = headers;
	info.layerCount = sizeof(headers) / sizeof(void*);

	OOVR_FAILED_XR_ABORT(xrEndFrame(xr_session, &info));
}

IBackend::openvr_enum_t XrBackend::SetSkyboxOverride(const vr::Texture_t* pTextures, uint32_t unTextureCount)
{
	STUBBED();
}

void XrBackend::ClearSkyboxOverride()
{
	STUBBED();
}

/* Misc compositor */

/**
 * Get frame timing information to be passed to the application
 *
 * Returns true if successful
 */
bool XrBackend::GetFrameTiming(OOVR_Compositor_FrameTiming* pTiming, uint32_t unFramesAgo)
{
	STUBBED();
}

/* D3D Mirror textures */
/* #if defined(SUPPORT_DX) */
IBackend::openvr_enum_t XrBackend::GetMirrorTextureD3D11(vr::EVREye eEye, void* pD3D11DeviceOrResource, void** ppD3D11ShaderResourceView)
{
	STUBBED();
}
void XrBackend::ReleaseMirrorTextureD3D11(void* pD3D11ShaderResourceView)
{
	STUBBED();
}
/* #endif */
/** Returns the points of the Play Area. */
bool XrBackend::GetPlayAreaPoints(vr::HmdVector3_t* points, int* count)
{
	STUBBED();
}
/** Determine whether the bounds are showing right now **/
bool XrBackend::AreBoundsVisible()
{
	STUBBED();
}
/** Set the boundaries to be visible or not (although setting this to false shouldn't affect
 * what happens if the player moves their hands too close and shows it that way) **/
void XrBackend::ForceBoundsVisible(bool status)
{
	STUBBED();
}
