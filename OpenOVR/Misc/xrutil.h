#pragma once

#include "OpenVR/interfaces/vrtypes.h"

#include <openxr/openxr.h>

// Note: Xru stands for openXR Utilities

/**
 * Unfortunately OpenXR doesn't define it's own eye constants, but since we know the order
 * for stereoscopic displays (only one of OpenXR's targets, hence why there's no enum for it) we
 * can just define our own.
 *
 * See OpenXR spec section 8.1
 * https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#view_configuration_type
 */
enum XruEye : int {
	XruEyeLeft = 0,
	XruEyeRight = 1,
	XruEyeCount = 2,
};

// A macro to ifndef against for a section being ported, so we can remove it to make sure we
// haven't missed anything
#define OC_XR_PORT

#define XR_STUBBED() \
	OOVR_ABORTF("Hit not-yet-ported-to-OpenXR stubbed function at %s:%d func %s", __FILE__, __LINE__, __func__);

#define LINUX_STUBBED() \
	OOVR_ABORTF("Hit not-yet-ported-to-Linux stubbed function at %s:%d func %s", __FILE__, __LINE__, __func__);

// Useful maths typedefs
// NOTE: THESE ARE DEPRECATED, REMOVE, AND USE THE GLM NAMES
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
typedef glm::mat4 MfMatrix4f;
typedef glm::vec3 MfVector3f;

class XrSessionGlobals {
public:
	XrSessionGlobals();

	XrSpace floorSpace;
	XrSpace seatedSpace;
	XrSpace viewSpace;

	// Set by XrBackend
	XrTime nextPredictedFrameTime = 0;

	/**
	 * The latest time we've observed from the runtime. This will be set before a frame is submitted, so for
	 * stuff that needs a time (but it probably doesn't matter much) this can be used.
	 *
	 * For stuff where the next frame time is ideal but some valid time is required, see GetBestTime().
	 *
	 * Note that due to poor implementation, this may lag a long way before nextPredictedFrameTime.
	 */
	XrTime latestTime = 0;

	/**
	 * Returns nextPredictedFrameTime if available, otherwise returns latestTime.
	 */
	XrTime GetBestTime();
};

XrSpace xr_space_from_tracking_origin(vr::ETrackingUniverseOrigin origin);

extern XrInstance xr_instance;
extern XrSession xr_session;
extern XrSessionState xr_session_state;
extern XrSystemId xr_system;
XrViewConfigurationView& xr_main_view(XruEye view_id);
extern XrSessionGlobals* xr_gbl;

class XrExt;
extern XrExt* xr_ext;
