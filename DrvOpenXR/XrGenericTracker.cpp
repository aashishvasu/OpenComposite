#include "XrGenericTracker.h"

#include "../OpenOVR/Misc/xrmoreutils.h"
#include "../OpenOVR/Reimpl/BaseInput.h"
#include "../OpenOVR/convert.h"

XrGenericTracker::XrGenericTracker(const InteractionProfile& profile, XrXDevPropertiesMNDX properties, uint32_t index, XrSpace space)
    : profile(profile), xdevProperties(properties), genericTrackerIndex(index), genericTrackerSpace(space)
{
	InitialiseDevice(index + RESERVED_DEVICE_INDICES);
}

XrGenericTracker::~XrGenericTracker()
{
	xrDestroySpace(genericTrackerSpace);
}

void XrGenericTracker::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState)
{
	ZeroMemory(pose, sizeof(*pose));
	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = false;
	pose->eTrackingResult = vr::TrackingResult_Running_OutOfRange;

	if (!genericTrackerSpace) {

		// Same workaround as XrController, unsure if needed for trackers. Better safe than sorry.
		pose->bPoseIsValid = true;
		pose->eTrackingResult = vr::TrackingResult_Running_OK;

		return;
	}

	xr_utils::PoseFromSpace(pose, genericTrackerSpace, origin, glm::identity<glm::mat4>());
}

// Properties

uint64_t XrGenericTracker::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (prop == vr::Prop_CurrentUniverseId_Uint64) {
		if (pErrorL)
			*pErrorL = vr::TrackedProp_Success;

		return 1; // Oculus Rift's universe
	}

	return XrTrackedDevice::GetUint64TrackedDeviceProperty(prop, pErrorL);
}

uint32_t XrGenericTracker::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
    char* value, uint32_t bufferSize, vr::ETrackedPropertyError* pErrorL)
{
#define PROP(in, out)                                                                                  \
	if (prop == in) {                                                                                  \
		if (value != NULL && bufferSize > 0) {                                                         \
			strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
		}                                                                                              \
		return (uint32_t)strlen(out) + 1;                                                              \
	}

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	PROP(vr::Prop_RenderModelName_String, GetInteractionProfile()->GetOpenVRName().value());
	PROP(vr::Prop_ControllerType_String, "vive_tracker_handheld_object");
	PROP(vr::Prop_ModelNumber_String, "Vive Tracker Handheld Object")
	PROP(vr::Prop_SerialNumber_String, xdevProperties.serial);

	return XrTrackedDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}

vr::ETrackedDeviceClass XrGenericTracker::GetTrackedDeviceClass()
{
	return vr::TrackedDeviceClass_GenericTracker;
}

const InteractionProfile* XrGenericTracker::GetInteractionProfile()
{
	return &profile;
}

ITrackedDevice::TrackedDeviceType XrGenericTracker::GetHand()
{
	return ITrackedDevice::TrackedDeviceType::GENERIC_TRACKER;
}
