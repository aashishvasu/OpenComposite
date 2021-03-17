#include "stdafx.h"

#include "xrutil.h"

#include "convert.h"
#include "xr_ext.h"

XrInstance xr_instance = XR_NULL_HANDLE;
XrSession xr_session = XR_NULL_HANDLE;
XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
XrSystemId xr_system = XR_NULL_SYSTEM_ID;
XrViewConfigurationView xr_main_views[XruEyeCount] = {};
XrSessionGlobals* xr_gbl = nullptr;

XrExt* xr_ext = nullptr;

std::vector<XrViewConfigurationView> xr_views_list{};
XrViewConfigurationView& xr_main_view(XruEye view_id)
{
	if (xr_views_list.empty()) {
		OOVR_ABORT("Cannot call xr_main_view before any views have been configured");
	}

	return xr_views_list.at(view_id);
}

XrExt::XrExt()
{
#define XR_BIND(name) OOVR_FAILED_XR_ABORT(xrGetInstanceProcAddr(xr_instance, #name, (PFN_xrVoidFunction*)&this->name))
#define XR_BIND_OPT(name) xrGetInstanceProcAddr(xr_instance, #name, (PFN_xrVoidFunction*)&this->name)

	XR_BIND_OPT(xrGetVisibilityMaskKHR);

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	XR_BIND(xrGetD3D11GraphicsRequirementsKHR);
#endif

#ifdef SUPPORT_VK
	XR_BIND(xrGetVulkanGraphicsRequirementsKHR);
	XR_BIND(xrGetVulkanInstanceExtensionsKHR);
	XR_BIND(xrGetVulkanDeviceExtensionsKHR);
	XR_BIND(xrGetVulkanGraphicsDeviceKHR);
#endif

#ifdef SUPPORT_GL
	XR_BIND(xrGetOpenGLGraphicsRequirementsKHR);
#endif

#undef XR_BIND
}

XrSessionGlobals::XrSessionGlobals()
{
	XrReferenceSpaceCreateInfo spaceInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };

	// No offset to the reference space
	// Maybe we should change this when the user re-centres?
	spaceInfo.poseInReferenceSpace.position = G2X_v3f(glm::vec3());
	spaceInfo.poseInReferenceSpace.orientation = G2X_quat(glm::identity<glm::quat>());

	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
	OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session, &spaceInfo, &floorSpace));

	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session, &spaceInfo, &seatedSpace));

	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session, &spaceInfo, &viewSpace));
}

XrTime XrSessionGlobals::GetBestTime()
{
	return nextPredictedFrameTime ? nextPredictedFrameTime : latestTime;
}

XrSpace xr_space_from_tracking_origin(vr::ETrackingUniverseOrigin origin)
{
	switch (origin) {
	case vr::TrackingUniverseSeated:
		return xr_gbl->seatedSpace;
	case vr::TrackingUniverseStanding:
		return xr_gbl->floorSpace;
	case vr::TrackingUniverseRawAndUncalibrated:
		OOVR_ABORT("Tracking origin TrackingUniverseRawAndUncalibrated not supported");
	default:
		OOVR_ABORTF("Unknown ETrackingUniverseOrigin type %d", origin);
	}
}
