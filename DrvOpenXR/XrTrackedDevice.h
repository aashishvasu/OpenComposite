//
// Created by ZNix on 25/10/2020.
//

#pragma once

#include "XrDriverPrivate.h"

class XrTrackedDevice : public virtual ITrackedDevice {
	void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState) override;

	void GetPose(
	    vr::ETrackingUniverseOrigin origin,
	    vr::TrackedDevicePose_t* pose,
	    ETrackingStateType trackingState,
	    double absTime) override;
};
