#pragma once

#include "XrTrackedDevice.h"

class XrController : public XrTrackedDevice {
public:
	enum XrControllerType : int {
		XCT_LEFT,
		XCT_RIGHT,
		XCT_TRACKED_OBJECT,
	};

	explicit XrController(XrControllerType type);

	HandType GetHand() override;

	void GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState) override;

	// properties
	bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	int32_t GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue,
	    uint32_t unBufferSize, vr::ETrackedPropertyError* pErrorL) override;

private:
	XrControllerType type;
};
