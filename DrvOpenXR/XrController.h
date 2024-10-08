#pragma once

#include "Misc/Input/InteractionProfile.h"
#include "XrTrackedDevice.h"

class BaseInput;

class XrController : public XrTrackedDevice {
public:
	enum XrControllerType : int {
		XCT_LEFT,
		XCT_RIGHT,
		XCT_TRACKED_OBJECT,
	};

	explicit XrController(XrControllerType type, const InteractionProfile& profile);

	TrackedDeviceType GetHand() override;

	void GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState) override;
	bool GetPoseFromHandTracking(BaseInput* input, vr::TrackedDevicePose_t* pose);

	vr::ETrackedDeviceClass GetTrackedDeviceClass() override;

	const InteractionProfile* GetInteractionProfile() override;

	// properties
	bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	int32_t GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue,
	    uint32_t unBufferSize, vr::ETrackedPropertyError* pErrorL) override;

	bool IsHandTrackingValid() override;

private:
	XrControllerType type;
	const InteractionProfile& profile;
	bool isHandTrackingValid = false;
};
