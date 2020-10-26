//
// Created by ZNix on 25/10/2020.
//

#include "XrTrackedDevice.h"

#include "../OpenOVR/convert.h"

void XrTrackedDevice::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState)
{
	// TODO everything!
	memset(pose, 0, sizeof(*pose));
	pose->bPoseIsValid = true;
	pose->bDeviceIsConnected = true;
	pose->eTrackingResult = vr::TrackingResult_Running_OK;
	pose->mDeviceToAbsoluteTracking = G2S_m34(glm::mat4());
#ifndef OC_XR_PORT
#error todo
#endif

	static int i = 0;
	pose->mDeviceToAbsoluteTracking.m[2][3] = (i++) * 0.05f;
}

void XrTrackedDevice::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState, double absTime)
{
	STUBBED();
}
