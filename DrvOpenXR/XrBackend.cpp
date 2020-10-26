//
// Created by ZNix on 25/10/2020.
//

#include "XrBackend.h"

IHMD* XrBackend::GetPrimaryHMD()
{
	return hmd.get();
}

ITrackedDevice* XrBackend::GetDevice(
    vr::TrackedDeviceIndex_t index)
{
	STUBBED();
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
void XrBackend::WaitForTrackingData()
{
	XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState state{ XR_TYPE_FRAME_STATE };

	OOVR_FAILED_XR_ABORT(xrWaitFrame(xr_session, &waitInfo, &state));

	// FIXME loop until this returns true?
	OOVR_FALSE_ABORT(state.shouldRender);

	XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	OOVR_FAILED_XR_ABORT(xrBeginFrame(xr_session, &beginInfo));
}

void XrBackend::StoreEyeTexture(
    vr::EVREye eye,
    const vr::Texture_t* texture,
    const vr::VRTextureBounds_t* bounds,
    vr::EVRSubmitFlags submitFlags,
    bool isFirstEye)
{
	std::unique_ptr<Compositor>& compPtr = compositors[eye];
	OOVR_FALSE_ABORT(compPtr.get() != nullptr);
	Compositor& comp = *compPtr;

	XrCompositionLayerProjectionView view{};
	comp.Invoke((XruEye)eye, texture, bounds, submitFlags, view);

	// TODO store view somewhere and use it for submitting our frame
}

void XrBackend::SubmitFrames(bool showSkybox)
{
	// TODO without this we don't send any frames to the headset
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
