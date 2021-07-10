#include "DrvOculusCommon.h"
#include "OculusDevice.h"
#include "../OpenOVR/libovr_wrapper.h"
#include "OculusBackend.h"

// TODO remove
#include "../OpenOVR/stdafx.h"
#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "../OpenOVR/convert.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

OculusDevice::OculusDevice(OculusBackend * backend)
	: backend(backend) {
}

OculusDevice::~OculusDevice() {
}

void OculusDevice::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t *pose, ETrackingStateType trackingState) {
	GetPose(origin, pose, trackingState, 0);
}

void OculusDevice::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t *pose, ETrackingStateType trackingState,
	double absTime) {
	ovrTrackingState state;
	if (trackingState == TrackingStateType_Now) {
		state = ovr_GetTrackingState(*ovr::session, 0 /* Most recent */, ovrTrue);
	}
	else if (trackingState == TrackingStateType_Prediction) {
		state = ovr_GetTrackingState(*ovr::session, absTime, ovrTrue);
	}
	else {
		state = backend->GetTrackingState();
	}

	GetPose(origin, pose, state);
}

void OculusDevice::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t * pose, const ovrTrackingState & state) {
	memset(pose, 0, sizeof(vr::TrackedDevicePose_t));

	ovrPoseStatef ovrPose;

	/*
	if (index == vr::k_unTrackedDeviceIndex_Hmd) {
		ovrPose = state.HeadPose;
	}
	else if (index == BaseSystem::leftHandIndex || index == BaseSystem::rightHandIndex) {
		ovrPose = state.HandPoses[index == BaseSystem::leftHandIndex ? ovrHand_Left : ovrHand_Right];
	}
	else if (index == BaseSystem::thirdTouchIndex) {
		ovrTrackedDeviceType type = ovrTrackedDevice_Object0;
		ovr_GetDevicePoses(*ovr::session, &type, 1, 0, &ovrPose);
	}
	else {
		pose->bPoseIsValid = false;
		pose->bDeviceIsConnected = false;
		return;
	}
	*/
	ovrPose = GetOculusPose(state);

	// If we haven't yet got a frame, mark the controller as having
	// an invalid pose to avoid errors from unnormalised 0,0,0,0 quaternions
	if (!ovrPose.TimeInSeconds) {
		pose->bPoseIsValid = false;
		return;
	}

	ovrPosef transform = GetOffset();
	ovrPose.ThePose = Posef(ovrPose.ThePose) * transform;

	// AFAIK we don't need to do anything like the above for the third Touch controller, since it
	//  isn't used as a controller anyway but rather a tracking device.

	// Configure the pose

	pose->bPoseIsValid = true;

	// TODO deal with the HMD not being connected
	pose->bDeviceIsConnected = true;

	// TODO
	pose->eTrackingResult = TrackingResult_Running_OK;

	O2S_v3f(ovrPose.LinearVelocity, pose->vVelocity);
	O2S_v3f(ovrPose.AngularVelocity, pose->vAngularVelocity);

	pose->mDeviceToAbsoluteTracking = GetUnsafeBaseSystem()->_PoseToTrackingSpace(origin, ovrPose.ThePose);
}

uint64_t OculusDevice::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (prop == vr::Prop_CurrentUniverseId_Uint64) {
		if (pErrorL)
			*pErrorL = vr::TrackedProp_Success;

		return 1; // Oculus Rift's universe
	}

	return ITrackedDevice::GetUint64TrackedDeviceProperty(prop, pErrorL);
}

uint32_t OculusDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
	char * value, uint32_t bufferSize, vr::ETrackedPropertyError * pErrorL) {

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

#define PROP(in, out) \
if(prop == in) { \
	if (value != NULL && bufferSize > 0) { \
		strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
	} \
	return (uint32_t) strlen(out) + 1; \
}

	// These have been validated against SteamVR
	// TODO add an option to fake this out with 'lighthouse' and 'HTC' in case there is a compatibility issue
	PROP(Prop_TrackingSystemName_String, "oculus");
	PROP(Prop_ManufacturerName_String, "Oculus");

	// TODO these?
	PROP(Prop_SerialNumber_String, "<unknown>"); // TODO
	PROP(Prop_RenderModelName_String, "<unknown>"); // It appears this just gets passed into IVRRenderModels as the render model name

	// Used by Firebird The Unfinished - see #58
	// Copied from SteamVR
	PROP(Prop_DriverVersion_String, "1.32.0");

#undef PROP

	return ITrackedDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}

ovrPosef OculusDevice::GetOffset() {
	return Posef::Identity();
}

////////////////////////////////
//// OculusControllerDevice ////
////////////////////////////////

OculusControllerDevice::OculusControllerDevice(OculusBackend *backend, EOculusTrackedObject device)
	: OculusDevice(backend), device(device) {
}

bool OculusControllerDevice::IsConnected() {
	unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);

	return (connected & GetControllerType()) != 0;
}

bool OculusControllerDevice::GetControllerState(vr::VRControllerState_t *state) {

	ovrHandType id;
	if (device == EOculusTrackedObject::LTouch) {
		id = ovrHand_Left;
	} else if (device == EOculusTrackedObject::RTouch) {
		id = ovrHand_Right;
	} else {
		return false;
	}

	uint64_t Buttons = 0;
	uint64_t Touches = 0;

	// TODO cache this
	ovrInputState inputState;
	ovrResult result = ovr_GetInputState(*ovr::session, ovrControllerType_Touch, &inputState);
	if (!OVR_SUCCESS(result)) {
		OOVR_LOGF("[WARN] Could not get input: %u", result);
		return false;
	}

#define CHECK(var, type, left, right, out) \
if(inputState.var & (id == ovrHand_Left ? ovr ## type ## _ ## left : ovr ## type ## _ ## right)) \
	var |= ButtonMaskFromId(out)

#define BUTTON(left, right, out) CHECK(Buttons, Button, left, right, out); CHECK(Touches, Touch, left, right, out)

	BUTTON(Y, B, k_EButton_ApplicationMenu);
	BUTTON(X, A, k_EButton_A); // k_EButton_A is the SteamVR name for the lower buttons on the Touch controllers
	BUTTON(LThumb, RThumb, k_EButton_SteamVR_Touchpad);
	// TODO

#undef BUTTON
#undef CHECK

	// Menu button
	if (id == ovrHand_Left && inputState.Buttons & ovrButton_Enter)
		Buttons |= ButtonMaskFromId(EVRButtonId::k_EButton_System);

	// Grip/Trigger button
	// TODO what should the cutoff be?
	if (inputState.HandTrigger[id] >= 0.4) {
		Buttons |= ButtonMaskFromId(k_EButton_Grip);
		Buttons |= ButtonMaskFromId(k_EButton_Axis2);
	}
	if (inputState.IndexTrigger[id] >= 0.4) {
		Buttons |= ButtonMaskFromId(k_EButton_SteamVR_Trigger);
	}

	if (inputState.Touches & (id == ovrHand_Left ? ovrTouch_LIndexTrigger : ovrTouch_RIndexTrigger)) {
		Touches |= ButtonMaskFromId(k_EButton_SteamVR_Trigger);
	}

	// Trigger and Thumbstick - Analog (axis) inputs
	VRControllerAxis_t &trigger = state->rAxis[1];
	trigger.x = inputState.IndexTrigger[id];
	trigger.y = 0;

	VRControllerAxis_t &grip = state->rAxis[2];
	grip.x = inputState.HandTrigger[id];
	grip.y = 0;

	VRControllerAxis_t &thumbstick = state->rAxis[0];
	ovrVector2f &ovrThumbstick = inputState.Thumbstick[id];
	thumbstick.x = ovrThumbstick.x;
	thumbstick.y = ovrThumbstick.y;

	// Pythagoras, and don't bother square rooting it since that's much slower than squaring what we compare it to
	float valueSquared = thumbstick.x * thumbstick.x + thumbstick.y * thumbstick.y;

	// The threshold for activating the virtual DPad buttons
	// TODO add a latch thing so you can't have it flip back and forth
	float threshold = 0.6f;

	if (valueSquared > threshold * threshold) {
		// 0=west
		float angle = atan2(thumbstick.y, thumbstick.x);

		// Subtract 45deg so the divisions are diagonal
		angle -= math_pi / 4;

		if (angle < 0)
			angle += math_pi * 2;

		if (angle < math_pi * 0.5) {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Right);
		}
		else if (angle < math_pi * 1.0) {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Down);
		}
		else if (angle < math_pi * 1.5) {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Left);
		}
		else {
			Buttons |= ButtonMaskFromId(k_EButton_DPad_Up);
		}
	}

	state->ulButtonPressed = Buttons;
	state->ulButtonTouched = Touches;

	return true;
}

ovrPoseStatef OculusControllerDevice::GetOculusPose(const ovrTrackingState & trackingState) {
	switch (device) {
	case EOculusTrackedObject::LTouch:
		return trackingState.HandPoses[ovrHand_Left];
	case EOculusTrackedObject::RTouch:
		return trackingState.HandPoses[ovrHand_Right];
	case EOculusTrackedObject::Object0: {
		ovrPoseStatef ovrPose;
		ovrTrackedDeviceType type = ovrTrackedDevice_Object0;
		ovr_GetDevicePoses(*ovr::session, &type, 1, 0, &ovrPose);
		return ovrPose;
	}
	default:
		OOVR_ABORTF("Unknown controller device %d", device);
	}
}

ovrPosef OculusControllerDevice::GetOffset() {
	// Use the same behaviour as before, not rotating the tracked 3rd controller
	if (device != EOculusTrackedObject::LTouch && device != EOculusTrackedObject::RTouch) {
		return Posef::Identity();
	}

	static Posef transform = Posef(Quatf(BaseCompositor::GetHandTransform()), BaseCompositor::GetHandTransform().GetTranslation());
	return transform;
}

bool OculusControllerDevice::IsTouchController() {
	return device == EOculusTrackedObject::LTouch || device == EOculusTrackedObject::RTouch;
}

ovrControllerType OculusControllerDevice::GetControllerType() {
	switch (device) {
	case EOculusTrackedObject::LTouch:
		return ovrControllerType_LTouch;
	case EOculusTrackedObject::RTouch:
		return ovrControllerType_RTouch;
	case EOculusTrackedObject::Object0:
		return ovrControllerType_Object0;
	default:
		OOVR_ABORTF("Unknown controller device %d", device);
	}
}

int32_t OculusControllerDevice::TriggerHapticVibrationAction(float fFrequency, float fAmplitude) {
	return ovr_SetControllerVibration(*ovr::session, GetControllerType(), fFrequency, fAmplitude);
}

// properties
bool OculusControllerDevice::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if(pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case Prop_DeviceProvidesBatteryStatus_Bool:
		return true;
	}

	return OculusDevice::GetBoolTrackedDeviceProperty(prop, pErrorL);
}

int32_t OculusControllerDevice::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	// Don't apply the inputs to the tracking object
	if (IsTouchController()) {
		switch (prop) {
		case Prop_Axis0Type_Int32:
			// TODO find out which of these SteamVR returns and do likewise
			//return k_eControllerAxis_TrackPad;
			return k_eControllerAxis_Joystick;

		case Prop_Axis1Type_Int32:
			return k_eControllerAxis_Trigger;

		case Prop_Axis2Type_Int32:
			return k_eControllerAxis_Trigger;

		case Prop_Axis3Type_Int32:
		case Prop_Axis4Type_Int32:
			return k_eControllerAxis_None;
		}
	}

	return OculusDevice::GetInt32TrackedDeviceProperty(prop, pErrorL);
}

uint64_t OculusControllerDevice::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	if (IsTouchController() && prop == Prop_SupportedButtons_Uint64) {
		return
			ButtonMaskFromId(k_EButton_ApplicationMenu) |
			ButtonMaskFromId(k_EButton_Grip) |
			ButtonMaskFromId(k_EButton_Axis2) |
			ButtonMaskFromId(k_EButton_DPad_Left) |
			ButtonMaskFromId(k_EButton_DPad_Up) |
			ButtonMaskFromId(k_EButton_DPad_Down) |
			ButtonMaskFromId(k_EButton_DPad_Right) |
			ButtonMaskFromId(k_EButton_A) |
			ButtonMaskFromId(k_EButton_SteamVR_Touchpad) |
			ButtonMaskFromId(k_EButton_SteamVR_Trigger);
	}

	return OculusDevice::GetUint64TrackedDeviceProperty(prop, pErrorL);
}

uint32_t OculusControllerDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
	char * value, uint32_t bufferSize, vr::ETrackedPropertyError * pErrorL) {

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

#define PROP(in, out) \
if(prop == in) { \
	if (value != NULL && bufferSize > 0) { \
		strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
	} \
	return (uint32_t) strlen(out) + 1; \
}

	switch (device) {
	case EOculusTrackedObject::LTouch:
		PROP(Prop_RenderModelName_String, "renderLeftHand");
		PROP(Prop_ModelNumber_String, "Oculus Rift CV1 (Left Controller)");
			PROP(Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F_Controller_Left"); // TODO is this different CV1 vs S?
		break;
	case EOculusTrackedObject::RTouch:
		PROP(Prop_RenderModelName_String, "renderRightHand");
		PROP(Prop_ModelNumber_String, "Oculus Rift CV1 (Right Controller)");
		PROP(Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F_Controller_Right");
		break;
	case EOculusTrackedObject::Object0:
		PROP(Prop_RenderModelName_String, "renderObject0");

		// This is made up, and not at all verified with SteamVR
		PROP(Prop_ModelNumber_String, "Oculus Rift CV1 (Tracked Object 0)");
		break;
	}

	PROP(Prop_ControllerType_String, "oculus_touch");

	return OculusDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}
