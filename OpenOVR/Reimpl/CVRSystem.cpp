#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("System", "009")
GEN_INTERFACE("System", "011")
GEN_INTERFACE("System", "012")
//
GEN_INTERFACE("System", "014")
GEN_INTERFACE("System", "015")
GEN_INTERFACE("System", "016")
GEN_INTERFACE("System", "017")
// AFAIK 18 doesn't exist
GEN_INTERFACE("System", "019")
GEN_INTERFACE("System", "020")
GEN_INTERFACE("System", "021")
GEN_INTERFACE("System", "022")

#include "generated/GVRSystem.gen.h"

// Old methods that don't take size arguments
bool CVRSystem_009::PollNextEvent(vr::VREvent_t* pEvent)
{
	return base->PollNextEvent(pEvent, sizeof(vr::VREvent_t));
}

bool CVRSystem_009::PollNextEventWithPose(vr::ETrackingUniverseOrigin eOrigin, vr::VREvent_t* pEvent, vr::TrackedDevicePose_t* pTrackedDevicePose)
{
	return base->PollNextEventWithPose(eOrigin, pEvent, sizeof(vr::VREvent_t), pTrackedDevicePose);
}

bool CVRSystem_009::GetControllerState(vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t* pControllerState)
{
	return base->GetControllerState(unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t));
}

bool CVRSystem_009::GetControllerStateWithPose(vr::ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
    vr::VRControllerState_t* pControllerState, vr::TrackedDevicePose_t* pTrackedDevicePose)
{
	return base->GetControllerStateWithPose(eOrigin, unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t), pTrackedDevicePose);
}

bool CVRSystem_011::GetControllerState(vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t* pControllerState)
{
	return base->GetControllerState(unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t));
}

bool CVRSystem_011::GetControllerStateWithPose(vr::ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
    vr::VRControllerState_t* pControllerState, vr::TrackedDevicePose_t* pTrackedDevicePose)
{
	return base->GetControllerStateWithPose(eOrigin, unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t), pTrackedDevicePose);
}

bool CVRSystem_012::GetControllerState(vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t* pControllerState)
{
	return base->GetControllerState(unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t));
}

bool CVRSystem_012::GetControllerStateWithPose(vr::ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
    vr::VRControllerState_t* pControllerState, vr::TrackedDevicePose_t* pTrackedDevicePose)
{
	return base->GetControllerStateWithPose(eOrigin, unControllerDeviceIndex, pControllerState, sizeof(vr::VRControllerState001_t), pTrackedDevicePose);
}
