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

	virtual void GetPose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDevicePose_t* pose,
		ETrackingStateType trackingState,
		double absTime) override;

	virtual bool IsConnected() = 0;

	// For optimisation
	void GetPose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDevicePose_t* pose,
		const ovrTrackingState &trackingState);

	virtual uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL);
	virtual uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue,
		uint32_t unBufferSize, vr::ETrackedPropertyError *pErrorL);

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

	virtual bool GetControllerState(vr::VRControllerState_t *state) override;

	virtual int32_t TriggerHapticVibrationAction(float fFrequency, float fAmplitude) override;

	// properties
	virtual bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL) override;
	virtual int32_t GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL) override;
	virtual uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pErrorL) override;
	virtual uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue,
		uint32_t unBufferSize, vr::ETrackedPropertyError *pErrorL) override;

protected:
	virtual ovrPoseStatef GetOculusPose(const ovrTrackingState &trackingState) override;

	virtual ovrPosef GetOffset() override;

	// Is this a controller, as opposed to a tracking controller?
	bool IsTouchController();

	ovrControllerType GetControllerType();

private:
	EOculusTrackedObject device;
};
