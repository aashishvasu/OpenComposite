#include "XrController.h"

// HACK: grab the pose from BaseInput
#include "../OpenOVR/Misc/xrmoreutils.h"
#include "../OpenOVR/Reimpl/BaseInput.h"
#include "generated/static_bases.gen.h"

XrController::XrController(XrController::XrControllerType type, const InteractionProfile& profile)
    : type(type), profile(profile)
{
	InitialiseDevice(GetHand() + 1);
}

#define TRY_PROFILE_PROP(type)                                                \
	do {                                                                      \
		std::optional<type> ret = profile.GetProperty<type>(prop, GetHand()); \
		if (ret.has_value())                                                  \
			return *ret;                                                      \
	} while (0)

// properties
bool XrController::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	TRY_PROFILE_PROP(bool);

	switch (prop) {
	case vr::Prop_DeviceProvidesBatteryStatus_Bool:
		return true;
	default:
		return XrTrackedDevice::GetBoolTrackedDeviceProperty(prop, pErrorL);
	}
}

int32_t XrController::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	TRY_PROFILE_PROP(int32_t);

	// Continue to pretend to be a CV1
	// Don't apply the inputs to the tracking object
	if (type == XCT_LEFT || type == XCT_RIGHT) {
		switch (prop) {
		case vr::Prop_Axis0Type_Int32:
			// TODO find out which of these SteamVR returns and do likewise
			// return vr::k_eControllerAxis_TrackPad;
			return vr::k_eControllerAxis_Joystick;

		case vr::Prop_Axis1Type_Int32:
		case vr::Prop_Axis2Type_Int32:
			return vr::k_eControllerAxis_Trigger;

		case vr::Prop_Axis3Type_Int32:
		case vr::Prop_Axis4Type_Int32:
			return vr::k_eControllerAxis_None;

		default:
			break;
		}
	}

	return XrTrackedDevice::GetInt32TrackedDeviceProperty(prop, pErrorL);
}

uint64_t XrController::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	TRY_PROFILE_PROP(uint64_t);

	// This is for the old input system, which we don't initially need
	if (prop == vr::Prop_SupportedButtons_Uint64) {
		// Just assume we're an Oculus Touch-style controller and enable all the buttons.
		uint64_t supported = 0;
		supported |= vr::ButtonMaskFromId(vr::k_EButton_System);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_Axis2);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_DPad_Left);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_DPad_Up);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_DPad_Down);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_DPad_Right);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_A);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
		supported |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
		return supported;
	}

	return XrTrackedDevice::GetUint64TrackedDeviceProperty(prop, pErrorL);
}

uint32_t XrController::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
    char* value, uint32_t bufferSize, vr::ETrackedPropertyError* pErrorL)
{

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	std::optional<std::string> ret = profile.GetProperty<std::string>(prop, GetHand());
	if (ret.has_value()) {
		if (value != NULL && bufferSize > 0) {
			strcpy_s(value, bufferSize, ret->c_str());
		}
		return ret->size() + 1;
	}

#define PROP(in, out)                                                                                  \
	if (prop == in) {                                                                                  \
		if (value != NULL && bufferSize > 0) {                                                         \
			strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
		}                                                                                              \
		return (uint32_t)strlen(out) + 1;                                                              \
	}

	// Resonite determines controller type by using render model name, if it can't recognize - it will load generic controller
	switch (type) {
	case XCT_LEFT: {
		std::optional<const char*> leftHandPath = GetInteractionProfile()->GetLeftHandRenderModelName();
		if (leftHandPath.has_value()) {
			PROP(vr::Prop_RenderModelName_String, leftHandPath.value());
		} else {
			PROP(vr::Prop_RenderModelName_String, "renderLeftHand");
		}
		PROP(vr::Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F_Controller_Left");
		break;
	}
	case XCT_RIGHT: {
		std::optional<const char*> rightHandPath = GetInteractionProfile()->GetRightHandRenderModelName();
		if (rightHandPath.has_value()) {
			PROP(vr::Prop_RenderModelName_String, rightHandPath.value());
		} else {
			PROP(vr::Prop_RenderModelName_String, "renderRightHand");
		}
		PROP(vr::Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F_Controller_Right");
		break;
	}
	case XCT_TRACKED_OBJECT:
		PROP(vr::Prop_RenderModelName_String, "renderObject0");
		break;
	default:
		OOVR_ABORTF("Invalid controller type %d", type);
	}

	return XrTrackedDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}

ITrackedDevice::HandType XrController::GetHand()
{
	switch (type) {
	case XCT_LEFT:
		return HAND_LEFT;
	case XCT_RIGHT:
		return HAND_RIGHT;
	default:
		return HAND_NONE;
	}
}

void XrController::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState)
{
	// Default to an invalid pose
	ZeroMemory(pose, sizeof(*pose));
	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = false;
	pose->eTrackingResult = vr::TrackingResult_Running_OutOfRange;

	BaseInput* input = GetUnsafeBaseInput();
	if (input == nullptr)
		return;

	// TODO do something with TrackingState
	XrSpace space = XR_NULL_HANDLE;

	// Specifically use grip pose, since that's what InteractionProfile::GetGripToSteamVRTransform uses
	GetBaseInput()->GetHandSpace(DeviceIndex(), space, false);

	if (!space) {
		// Lie and say the pose is valid if the actions haven't even been loaded yet.
		// This is a workaround for games like DCS, which appear to require valid poses before
		// it will even attempt to request the controller state (and thus create the actions).
		if (!input->AreActionsLoaded()) {
			pose->bPoseIsValid = true;
			pose->eTrackingResult = vr::TrackingResult_Running_OK;
		}
		return;
	}

	// Find the hand transform matrix, and include that
	glm::mat4 transform = profile.GetGripToSteamVRTransform(GetHand());

	xr_utils::PoseFromSpace(pose, space, origin, transform);
}

vr::ETrackedDeviceClass XrController::GetTrackedDeviceClass()
{
	return vr::TrackedDeviceClass_Controller;
}

const InteractionProfile* XrController::GetInteractionProfile()
{
	return &profile;
}
