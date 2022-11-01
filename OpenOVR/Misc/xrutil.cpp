#include "stdafx.h"

#include "xrutil.h"

#include "Drivers/Backend.h"
#include "convert.h"
#include "xr_ext.h"

XrInstance xr_instance = XR_NULL_HANDLE;
SessionWrapper xr_session;
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

XrExt::XrExt(XrGraphicsApiSupportedFlags apis, const std::vector<const char*>& extensions)
{
	// Check the extensions we have selected, and don't fetch functions if we're not allowed to use them
	bool hasVisMask = false;
	bool hasHandTracking = false;
	for (const char* ext : extensions) {
		if (strcmp(ext, XR_KHR_VISIBILITY_MASK_EXTENSION_NAME) == 0)
			hasVisMask = true;
		if (strcmp(ext, XR_EXT_HAND_TRACKING_EXTENSION_NAME) == 0)
			hasHandTracking = true;
		if (strcmp(ext, XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME) == 0)
			supportsG2Controller = true;
	}

#define XR_BIND(name, function) OOVR_FAILED_XR_ABORT(xrGetInstanceProcAddr(xr_instance, #name, (PFN_xrVoidFunction*)&this->function))
#define XR_BIND_OPT(name, function) xrGetInstanceProcAddr(xr_instance, #name, (PFN_xrVoidFunction*)&this->function)

	if (hasVisMask)
		XR_BIND_OPT(xrGetVisibilityMaskKHR, pfnXrGetVisibilityMaskKHR);

	if (hasHandTracking) {
		XR_BIND(xrCreateHandTrackerEXT, pfnXrCreateHandTrackerExt);
		XR_BIND(xrDestroyHandTrackerEXT, pfnXrDestroyHandTrackerExt);
		XR_BIND(xrLocateHandJointsEXT, pfnXrLocateHandJointsExt);
	}

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	if (apis & XR_SUPPORTED_GRAPHICS_API_D3D11) {
		XR_BIND(xrGetD3D11GraphicsRequirementsKHR, pfnXrGetD3D11GraphicsRequirementsKHR);
	}
#endif

#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)
	if (apis & XR_SUPPORTED_GRAPHICS_API_D3D12) {
		XR_BIND(xrGetD3D12GraphicsRequirementsKHR, pfnXrGetD3D12GraphicsRequirementsKHR);
	}
#endif

#ifdef SUPPORT_VK
	if (apis & XR_SUPPORTED_GRAPHICS_API_VK) {
		XR_BIND(xrGetVulkanGraphicsRequirementsKHR, pfnXrGetVulkanGraphicsRequirementsKHR);
		XR_BIND(xrGetVulkanInstanceExtensionsKHR, pfnXrGetVulkanInstanceExtensionsKHR);
		XR_BIND(xrGetVulkanDeviceExtensionsKHR, pfnXrGetVulkanDeviceExtensionsKHR);
		XR_BIND(xrGetVulkanGraphicsDeviceKHR, pfnXrGetVulkanGraphicsDeviceKHR);
	}
#endif

#ifdef SUPPORT_GL
	if (apis & XR_SUPPORTED_GRAPHICS_API_GL) {
		XR_BIND(xrGetOpenGLGraphicsRequirementsKHR, pfnXrGetOpenGLGraphicsRequirementsKHR);
	}
#endif

#ifdef SUPPORT_GLES
	if (apis & XR_SUPPORTED_GRAPHICS_API_GLES) {
		XR_BIND(xrGetOpenGLESGraphicsRequirementsKHR, pfnXrGetOpenGLESGraphicsRequirementsKHR);
	}
#endif

#undef XR_BIND
#undef XR_BIND_OPT
}

XrSessionGlobals::XrSessionGlobals()
{
	XrReferenceSpaceCreateInfo spaceInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };

	// No offset to the reference space
	// Maybe we should change this when the user re-centres?
	spaceInfo.poseInReferenceSpace.position = G2X_v3f(glm::vec3());
	spaceInfo.poseInReferenceSpace.orientation = G2X_quat(glm::identity<glm::quat>());

	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
	OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session.get(), &spaceInfo, &floorSpace));

	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session.get(), &spaceInfo, &seatedSpace));

	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	OOVR_FAILED_XR_ABORT(xrCreateReferenceSpace(xr_session.get(), &spaceInfo, &viewSpace));

	// Read the system properties, including those for hand-tracking
	if (xr_ext->handTrackingExtensionAvailable())
		systemProperties.next = &handTrackingProperties;
	OOVR_FAILED_XR_ABORT(xrGetSystemProperties(xr_instance, xr_system, &systemProperties));
}

XrTime XrSessionGlobals::GetBestTime()
{
	return nextPredictedFrameTime > 1 ? nextPredictedFrameTime : latestTime;
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

XrSpace xr_space_from_ref_space_type(XrReferenceSpaceType spaceType)
{
	switch (spaceType) {
	case XR_REFERENCE_SPACE_TYPE_VIEW:
		return xr_gbl->viewSpace;
	case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:
	case XR_REFERENCE_SPACE_TYPE_LOCAL:
		return xr_gbl->seatedSpace;
	case XR_REFERENCE_SPACE_TYPE_STAGE:
		return xr_gbl->floorSpace;
	default:
		OOVR_ABORTF("Unknown XrReferenceSpaceType type %d", spaceType);
	}
}

XrQuaternionf yRotation(XrQuaternionf& q)
{
	float theta = atan2(q.y, q.w);
	return { 0.f, sinf(theta), 0.f, cosf(theta) };
}

XrQuaternionf yRotation(XrQuaternionf& q, XrQuaternionf& r)
{
	float theta = atan2(q.y, q.w);
	theta += atan2(r.y, r.w);
	return { 0.f, sinf(theta), 0.f, cosf(theta) };
}

float v3_dot(const XrVector3f& a, const XrVector3f& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

XrVector3f v3_cross(const XrVector3f& a, const XrVector3f& b)
{
	return XrVector3f{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

XrVector3f operator*(float a, const XrVector3f& b)
{
	return { b.x * a, b.y * a, b.z * a };
}

XrVector3f operator+(const XrVector3f& a, const XrVector3f& b)
{
	return { b.x + a.x, b.y + a.y, b.z + a.z };
}

void rotate_vector_by_quaternion(const XrVector3f& v, const XrQuaternionf& q, XrVector3f& vprime)
{
	// Extract the vector part of the quaternion
	XrVector3f u{ q.x, q.y, q.z };

	// Extract the scalar part of the quaternion
	float s = q.w;

	// Do the math
	vprime = 2.0f * v3_dot(u, v) * u
	    + (s * s - v3_dot(u, u)) * v
	    + 2.0f * s * v3_cross(u, v);
}

XrSession& SessionWrapper::get()
{
	return SessionLock(*this, false);
}

SessionLock SessionWrapper::lock_shared()
{
	return SessionLock(*this, false);
}

SessionLock SessionWrapper::lock()
{
	return SessionLock(*this, true);
}

void SessionWrapper::reset()
{
	session = XR_NULL_HANDLE;
}

SessionLock::SessionLock(SessionWrapper& p, bool exclusive)
    : parent(p)
{
	if (!thread_owns_lock) {
		if (exclusive)
			lock = std::unique_lock(p.mutex);
		else
			shared_lock = std::shared_lock(p.mutex);

		thread_owns_lock = true;
		owner = true;
	}
}

SessionLock::~SessionLock()
{
	if (owner)
		thread_owns_lock = false;
}

SessionLock::operator XrSession&() { return parent.session; }
