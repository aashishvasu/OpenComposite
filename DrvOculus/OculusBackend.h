#pragma once
#include "../OpenOVR/Drivers/Backend.h"
#include "../OpenOVR/Compositor/compositor.h"

class OculusBackend : public IBackend {
public:

	OculusBackend();
	virtual ~OculusBackend() override;

	virtual void GetSinglePose(
		vr::ETrackingUniverseOrigin origin,
		vr::TrackedDeviceIndex_t index,
		vr::TrackedDevicePose_t* pose,
		ETrackingStateType trackingState) override;

	virtual void GetDeviceToAbsoluteTrackingPose(
		vr::ETrackingUniverseOrigin toOrigin,
		float predictedSecondsToPhotonsFromNow,
		vr::TrackedDevicePose_t * poseArray,
		uint32_t poseArrayCount) override;

};
