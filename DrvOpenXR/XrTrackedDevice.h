//
// Created by ZNix on 25/10/2020.
//

#pragma once

#include "XrDriverPrivate.h"

class XrTrackedDevice : public virtual ITrackedDevice {
public:
	void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState) override;

	void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState,
	    double absTime) override;

	uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL) override;
	uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError* pErrorL) override;
};
