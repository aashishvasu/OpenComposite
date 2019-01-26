#pragma once
#include <memory>
#include "OpenVR/interfaces/vrtypes.h"

enum ETrackingStateType {
	/**
	* Use the latest available tracking data
	*/
	TrackingStateType_Now,

	/**
	* Use the tracking data corresponding to the upcoming frame
	*/
	TrackingStateType_Rendering,
};

#define DECLARE_BACKEND_FUNCS(PREPEND, APPEND) \
PREPEND void GetSinglePose( \
	vr::ETrackingUniverseOrigin origin, \
	vr::TrackedDeviceIndex_t index, \
	vr::TrackedDevicePose_t* pose, \
	ETrackingStateType trackingState) APPEND; \
\
PREPEND void GetDeviceToAbsoluteTrackingPose( \
	vr::ETrackingUniverseOrigin toOrigin, \
	float predictedSecondsToPhotonsFromNow, \
	vr::TrackedDevicePose_t * poseArray, \
	uint32_t poseArrayCount) APPEND; \
 \
/* Submitting Frames */ \
PREPEND void WaitForTrackingData() APPEND; \
\
PREPEND void StoreEyeTexture( \
	vr::EVREye eye, \
	const vr::Texture_t * texture, \
	const vr::VRTextureBounds_t * bounds, \
	vr::EVRSubmitFlags submitFlags, \
	bool isFirstEye) APPEND; \
\
PREPEND void SubmitFrames(bool showSkybox) APPEND; \
\
PREPEND IBackend::openvr_enum_t SetSkyboxOverride(const vr::Texture_t * pTextures, uint32_t unTextureCount) APPEND; \
\
PREPEND void ClearSkyboxOverride() APPEND; \


/**
 * Backend is similar in concept to the OpenVR driver API, however it is
 * much closer to the application-facing API, and thus can avoid quite a bit of
 * the potential performance issues that arise when using the OpenVR driver API when
 * proxying to another high-level API (such as LibOVR or OpenHMD).
 *
 * Basically, it's a single interface class that can do anything a OpenVR driver can do. Due
 * to it's naturally close integration with OpenComposite, this API is unversioned.
 */
class IBackend {
public:
	typedef int openvr_enum_t;

	DECLARE_BACKEND_FUNCS(virtual, =0)

	// Virtual Destructor
	virtual ~IBackend();
};

class BackendManager {
public:
	static void Create(IBackend*);
	static BackendManager& Instance();
	static void Reset();

	// main methods

	DECLARE_BACKEND_FUNCS(,)

private:
	BackendManager();

	// Required for private dtor
	friend std::default_delete<BackendManager>;
	~BackendManager();

	static std::unique_ptr<BackendManager> instance;

	// For now, just use a single backend
	// and add multiple in later
	IBackend *backend;
};
