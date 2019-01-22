#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("ServerDriverHost", "005", DRIVER)

#include "GVRServerDriverHost.gen.h"

// for some reason, these aren't getting implemeneted automatically
bool CVRServerDriverHost_005::TrackedDeviceAdded(const char* pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, ITrackedDeviceServerDriver* pDriver) {
	return base->TrackedDeviceAdded(pchDeviceSerialNumber, eDeviceClass, pDriver);
}

void CVRServerDriverHost_005::TrackedDevicePoseUpdated(uint32_t unWhichDevice, const DriverPose_t& newPose, uint32_t unPoseStructSize) {
	base->TrackedDevicePoseUpdated(unWhichDevice, newPose, unPoseStructSize);
}
