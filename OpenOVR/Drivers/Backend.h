#pragma once
#include <memory>
#include "OpenVR/interfaces/vrtypes.h"
#include "../OpenOVR/custom_types.h" // TODO move this into the OpenVR tree

// for OOVR_Compositor_FrameTiming
#include "../OpenOVR/Reimpl/BaseCompositor.h"

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

/**
 * Represents a single physical device
 */
class ITrackedDevice {
public:
	virtual ~ITrackedDevice();

	virtual void GetPose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDevicePose_t* pose,
		ETrackingStateType trackingState) = 0;
};

/**
 * Represents a head-mounted display
 */
class IHMD : public virtual ITrackedDevice {
public:
	virtual void GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) = 0;

	// from BaseSystem

	/** The projection matrix for the specified eye */
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ) = 0;
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ, EGraphicsAPIConvention convention) = 0;

	/** The components necessary to build your own projection matrix in case your
	* application is doing something fancy like infinite Z */
	virtual void GetProjectionRaw(vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom) = 0;

	/** Gets the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in
	* the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport.
	* Returns true for success. Otherwise, returns false, and distortion coordinates are not suitable. */
	virtual bool ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t *pDistortionCoordinates) = 0;

	/** Returns the transform from eye space to the head space. Eye space is the per-eye flavor of head
	* space that provides stereo disparity. Instead of Model * View * Projection the sequence is Model * View * Eye^-1 * Projection.
	* Normally View and Eye^-1 will be multiplied together and treated as View in your application.
	*/
	virtual vr::HmdMatrix34_t GetEyeToHeadTransform(vr::EVREye eEye) = 0;

};

#define DECLARE_BACKEND_FUNCS(PREPEND, APPEND) \
PREPEND IHMD* GetPrimaryHMD() APPEND; \
\
PREPEND ITrackedDevice* GetDevice( \
	vr::TrackedDeviceIndex_t index) APPEND; \
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
\
/* Misc compositor */ \
\
/** \
 * Get frame timing information to be passed to the application \
 * \
 * Returns true if successful \
 */ \
PREPEND bool GetFrameTiming(OOVR_Compositor_FrameTiming * pTiming, uint32_t unFramesAgo) APPEND; \
\
/* D3D Mirror textures */ \
/* #if defined(SUPPORT_DX) */ \
PREPEND IBackend::openvr_enum_t GetMirrorTextureD3D11(vr::EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) APPEND; \
PREPEND void ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) APPEND; \
/* #endif */ \


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

	// Legacy, replaced by GetDevice(index)->GetPose(...)
	void BackendManager::GetSinglePose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDeviceIndex_t index,
		vr::TrackedDevicePose_t * pose,
		ETrackingStateType trackingState);

	static vr::TrackedDevicePose_t InvalidPose();

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
