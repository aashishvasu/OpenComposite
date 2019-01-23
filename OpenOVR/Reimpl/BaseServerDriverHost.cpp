#include "stdafx.h"
#define BASE_IMPL
#include "BaseServerDriverHost.h"

#include "Drivers/DriverManager.h"

bool BaseServerDriverHost::TrackedDeviceAdded(OCTrackedDeviceDriver *driver) {
	return DriverManager::CheckInstance().TrackedDeviceAdded(driver);
}
void BaseServerDriverHost::TrackedDevicePoseUpdated(uint32_t unWhichDevice, const OCDriverPose_t & newPose, uint32_t unPoseStructSize) {
	STUBBED();
}
void BaseServerDriverHost::VsyncEvent(double vsyncTimeOffsetSeconds) {
	STUBBED();
}
void BaseServerDriverHost::VendorSpecificEvent(uint32_t unWhichDevice, vr::EVREventType eventType, const vr::VREvent_Data_t & eventData, double eventTimeOffset) {
	STUBBED();
}
bool BaseServerDriverHost::IsExiting() {
	STUBBED();
}
bool BaseServerDriverHost::PollNextEvent(vr::VREvent_t *pEvent, uint32_t uncbVREvent) {
	STUBBED();
}
void BaseServerDriverHost::GetRawTrackedDevicePoses(float fPredictedSecondsFromNow, vr::TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {
	STUBBED();
}
void BaseServerDriverHost::TrackedDeviceDisplayTransformUpdated(uint32_t unWhichDevice, vr::HmdMatrix34_t eyeToHeadLeft, vr::HmdMatrix34_t eyeToHeadRight) {
	STUBBED();
}
