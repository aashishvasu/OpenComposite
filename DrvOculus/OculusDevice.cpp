#include "DrvOculusCommon.h"
#include "OculusDevice.h"
#include "../OpenOVR/libovr_wrapper.h"
#include "OculusBackend.h"

// TODO remove
#include "../OpenOVR/stdafx.h"
#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "../OpenOVR/convert.h"
#include "../OpenOVR/Misc/Config.h"

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

	return connected && GetControllerType() != 0;
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

///////////////////////////////
////////// OculusHMD //////////
///////////////////////////////

OculusHMD::OculusHMD(OculusBackend * backend) : OculusDevice(backend) {
}

bool OculusHMD::IsConnected() {
	return true;
}

void OculusHMD::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	ovrSizei size = ovr_GetFovTextureSize(
		*ovr::session,
		ovrEye_Left, // Resolutions are done per-eye in LibOVR, no particular reason for left eye
		ovr::hmdDesc.DefaultEyeFov[ovrEye_Left],
		oovr_global_configuration.SupersampleRatio()
	);

	*width = size.w;
	*height = size.h;
}

ovrPoseStatef OculusHMD::GetOculusPose(const ovrTrackingState & trackingState) {
	return trackingState.HeadPose;
}

ovrPosef OculusHMD::GetOffset() {
	return Posef::Identity();
}
