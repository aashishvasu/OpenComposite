#pragma once

#include "generated/interfaces/vrtypes.h"
#include <mutex>
#include <openxr/openxr.h>
#include <shared_mutex>
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

// Useful maths typ
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

	XrSystemProperties systemProperties = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesEXT handTrackingProperties = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };

	// Set by XrBackend
	XrTime nextPredictedFrameTime = 1;

	/**
	 * The latest time we've observed from the runtime. This will be set before a frame is submitted, so for
	 * stuff that needs a time (but it probably doesn't matter much) this can be used.
	 *
	 * For stuff where the next frame time is ideal but some valid time is required, see GetBestTime().
	 *
	 * Note that due to poor implementation, this may lag a long way before nextPredictedFrameTime.
	 *
	 * Zero or negative values for XrTime will result in XR_ERROR_TIME_INVALID so initialise to 1.
	 */
	XrTime latestTime = 1;

	/**
	 * Returns nextPredictedFrameTime if available, otherwise returns latestTime.
	 */
	XrTime GetBestTime();
};

class SessionLock;
// A wrapper around the XrSession to prevent session destruction while it's being accessed
class SessionWrapper {

	std::shared_mutex mutex;
	XrSession session = XR_NULL_HANDLE;
	friend SessionLock;

public:
	/*
	 * Gets a reference to the underlying XrSession that won't be changed while in use.
	 */
	XrSession& get();

	/*
	 * Provides a RAII-style lock on accesses to the XrSession. Prevents any other thread from obtaining get() or lock_shared().
	 */
	SessionLock lock();

	/*
	 * Provides a RAII-style lock on modification to the XrSession. Prevents any other thread from obtaining lock().
	 */
	[[nodiscard]] SessionLock lock_shared();

	/*
	 * Resets the underlying XrSession to XR_NULL_HANDLE. Does not actually call xrDestroySession.
	 */
	void reset();
};

// This class is to prevent having to litter mutexes everywhere.
// It gets a shared_lock or lock on the session mutex and the mutex gets unlocked (on destruction)
// after whatever function call needs the session completes.
class SessionLock {
	std::shared_lock<std::shared_mutex> shared_lock;
	std::unique_lock<std::shared_mutex> lock;
	SessionWrapper& parent;
	bool owner = false;
	inline static thread_local bool thread_owns_lock = false;

public:
	SessionLock(SessionWrapper& p, bool exclusive);
	~SessionLock();
	operator XrSession&();
};

XrSpace xr_space_from_tracking_origin(vr::ETrackingUniverseOrigin origin);
XrSpace xr_space_from_ref_space_type(XrReferenceSpaceType spaceType);

XrQuaternionf yRotation(XrQuaternionf& q, XrQuaternionf& r);
void rotate_vector_by_quaternion(const XrVector3f& v, const XrQuaternionf& q, XrVector3f& vprime);

extern XrInstance xr_instance;
extern SessionWrapper xr_session;
extern XrSystemId xr_system;
XrViewConfigurationView& xr_main_view(XruEye view_id);
extern XrSessionGlobals* xr_gbl;

class XrExt;
extern XrExt* xr_ext;
