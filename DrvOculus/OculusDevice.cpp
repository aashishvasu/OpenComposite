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
	ovrTrackingState state;
	if (trackingState == TrackingStateType_Now) {
		state = ovr_GetTrackingState(*ovr::session, 0 /* Most recent */, ovrTrue);
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
		break;
	case EOculusTrackedObject::RTouch:
		PROP(Prop_RenderModelName_String, "renderRightHand");
		PROP(Prop_ModelNumber_String, "Oculus Rift CV1 (Right Controller)");
		break;
	}

	// TODO render model and model number for the tracking object

	return OculusDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}
