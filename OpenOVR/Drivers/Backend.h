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

	virtual void GetSinglePose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDeviceIndex_t index,
		vr::TrackedDevicePose_t* pose,
		ETrackingStateType trackingState) = 0;

	virtual void GetDeviceToAbsoluteTrackingPose(
		vr::ETrackingUniverseOrigin toOrigin,
		float predictedSecondsToPhotonsFromNow,
		vr::TrackedDevicePose_t * poseArray,
		uint32_t poseArrayCount) = 0;

	// Submitting Frames
	virtual void WaitForTrackingData() = 0;

	virtual void StoreEyeTexture(
		vr::EVREye eye,
		const vr::Texture_t * texture,
		const vr::VRTextureBounds_t * bounds,
		vr::EVRSubmitFlags submitFlags,
		bool isFirstEye) = 0;

	virtual void SubmitFrames(bool showSkybox) = 0;

	virtual openvr_enum_t SetSkyboxOverride(const vr::Texture_t * pTextures, uint32_t unTextureCount) = 0;

	virtual void ClearSkyboxOverride() = 0;

	// Virtual Destructor
	virtual ~IBackend();
};

class BackendManager {
public:
	static void Create(IBackend*);
	static BackendManager& Instance();
	static void Reset();

	// main methods

	void GetSinglePose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDeviceIndex_t index,
		vr::TrackedDevicePose_t* pose,
		ETrackingStateType trackingState);

	void GetDeviceToAbsoluteTrackingPose(
		vr::ETrackingUniverseOrigin toOrigin,
		float predictedSecondsToPhotonsFromNow,
		vr::TrackedDevicePose_t * poseArray,
		uint32_t poseArrayCount);

	// Submitting Frames

	void WaitForTrackingData();

	void StoreEyeTexture(
		vr::EVREye eye,
		const vr::Texture_t * texture,
		const vr::VRTextureBounds_t * bounds,
		vr::EVRSubmitFlags submitFlags,
		bool isFirstEye);

	void SubmitFrames(bool showSkybox);

	IBackend::openvr_enum_t SetSkyboxOverride(const vr::Texture_t * pTextures, uint32_t unTextureCount);

	void ClearSkyboxOverride();

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
