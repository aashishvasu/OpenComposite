#pragma once
#include "../OpenOVR/Drivers/Backend.h"
#include "OVR_CAPI.h"

class OculusBackend;

class OculusDevice : public virtual ITrackedDevice {
public:
	OculusDevice(OculusBackend *backend);

	~OculusDevice() override;

	virtual void GetPose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDevicePose_t* pose,
		ETrackingStateType trackingState) override;

	virtual bool IsConnected() = 0;

	// For optimisation
	void GetPose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDevicePose_t* pose,
		const ovrTrackingState &trackingState);

protected:
	virtual ovrPoseStatef GetOculusPose(const ovrTrackingState &trackingState) = 0;

	virtual ovrPosef GetOffset();

	OculusBackend *backend;
};

enum class EOculusTrackedObject {
	LTouch,
	RTouch,
	Object0,
};

class OculusControllerDevice : public OculusDevice {
public:
	OculusControllerDevice(OculusBackend *backend, EOculusTrackedObject device);

	// delete the default ctors
	OculusControllerDevice() = delete;
	OculusControllerDevice(OculusControllerDevice&) = delete;

	virtual bool IsConnected() override;

protected:
	virtual ovrPoseStatef GetOculusPose(const ovrTrackingState &trackingState) override;

	virtual ovrPosef GetOffset() override;

	ovrControllerType GetControllerType();

private:
	EOculusTrackedObject device;
};

// "using OculusDevice::GetPose;" isn't recognised by MSVC for some reason, so pragma it out
#pragma warning(push)
#pragma warning(disable: 4250)
class OculusHMD : public OculusDevice, public IHMD {
public:
	OculusHMD(OculusBackend *backend);

	// delete the default ctors
	OculusHMD() = delete;
	OculusHMD(OculusHMD&) = delete;

	virtual bool IsConnected() override;

	using OculusDevice::GetPose;

	virtual void GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) override;

	// from BaseSystem

	/** The projection matrix for the specified eye */
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ) override;
	virtual vr::HmdMatrix44_t GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ, EGraphicsAPIConvention convention) override;

	/** The components necessary to build your own projection matrix in case your
	* application is doing something fancy like infinite Z */
	virtual void GetProjectionRaw(vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom) override;

	/** Gets the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in
	* the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport.
	* Returns true for success. Otherwise, returns false, and distortion coordinates are not suitable. */
	virtual bool ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t *pDistortionCoordinates) override;

	/** Returns the transform from eye space to the head space. Eye space is the per-eye flavor of head
	* space that provides stereo disparity. Instead of Model * View * Projection the sequence is Model * View * Eye^-1 * Projection.
	* Normally View and Eye^-1 will be multiplied together and treated as View in your application.
	*/
	virtual vr::HmdMatrix34_t GetEyeToHeadTransform(vr::EVREye eEye) override;

protected:
	virtual ovrPoseStatef GetOculusPose(const ovrTrackingState &trackingState) override;

	virtual ovrPosef GetOffset() override;
};
#pragma warning(pop)
