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

protected:
	virtual ovrPoseStatef GetOculusPose(const ovrTrackingState &trackingState) override;

	virtual ovrPosef GetOffset() override;
};
#pragma warning(pop)
