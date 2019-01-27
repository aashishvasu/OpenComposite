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

// from BaseSystem

HmdMatrix44_t OculusHMD::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	return GetProjectionMatrix(eye, znear, zfar, API_DirectX);
}

HmdMatrix44_t OculusHMD::GetProjectionMatrix(EVREye eye, float znear, float zfar, EGraphicsAPIConvention convention) {
	ovrMatrix4f matrix = ovrMatrix4f_Projection(
		ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)],
		znear, zfar,
		convention == API_OpenGL ? ovrProjection_ClipRangeOpenGL : ovrProjection_None // TODO is this right?
	);

	return O2S_m4(matrix);
}

void OculusHMD::GetProjectionRaw(EVREye eye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
	/**
	* With a straight passthrough:
	*
	* SteamVR Left:  -1.110925, 0.889498, -0.964926, 0.715264
	* SteamVR Right: -1.110925, 0.889498, -0.715264, 0.964926
	* OpenOVR Left:  0.889498, 1.110925, 0.964926, 0.715264
	* OpenOVR Right: 0.889498, 1.110925, 0.715264, 0.964926
	*
	* Via:
	*   char buff[1024];
	*   snprintf(buff, sizeof(buff), "eye=%d %f, %f, %f, %f", eye, *pfTop, *pfBottom, *pfLeft, *pfRight);
	*   OOVR_LOG(buff);
	*
	* This suggests that SteamVR negates the top and left values. We should do that too, for obvious reasons.
	*/

	ovrFovPort fov = ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)];
	*pfTop = -fov.DownTan; // negate, and for some reason the up and down have to be switched
	*pfBottom = fov.UpTan;
	*pfLeft = -fov.LeftTan; // negate
	*pfRight = fov.RightTan;
}

bool OculusHMD::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * out) {
	// Here's what SteamVR does when run on a Rift:
	//  each of the output values match the input values, for all colour channels, regardless of the eye.
	// This is thus essentially a identity transform function.

	if (!out)
		return true;

	out->rfRed[0] = out->rfGreen[0] = out->rfBlue[0] = fU;
	out->rfRed[1] = out->rfGreen[1] = out->rfBlue[1] = fV;

	return true;
}

HmdMatrix34_t OculusHMD::GetEyeToHeadTransform(EVREye ovr_eye) {
	ovrEyeType eye = S2O_eye(ovr_eye);
	ovrPosef &pose = ovr::hmdToEyeViewPose[eye];

	OVR::Matrix4f transform(pose);
	// For some bizzare reason, inverting the matrix (to go from hmd->eye
	// to eye->hmd) breaks the view, and it's fine without it. That or I'm misunderstanding
	// what exactly this method is supposed to return.

	HmdMatrix34_t result;
	O2S_om34(transform, result);
	return result;
}
