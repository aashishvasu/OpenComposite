#include "DrvOculusCommon.h"
#include "OculusBackend.h"

// TODO move into OCOVR
#include "../OpenOVR/stdafx.h"
#include "../OpenOVR/libovr_wrapper.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "../OpenOVR/convert.h"
#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/Reimpl/BaseCompositor.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

using namespace vr;

OculusBackend::OculusBackend() {
	memset(&trackingState, 0, sizeof(ovrTrackingState));
}

OculusBackend::~OculusBackend() {
	for (int eye = 0; eye < 2; eye++) {
		if (compositors[eye])
			delete compositors[eye];
	}
}

static void GetSingleTrackingPose(
	ETrackingUniverseOrigin origin,
	TrackedDeviceIndex_t index,
	TrackedDevicePose_t* pose,
	ovrTrackingState &state) {

	memset(pose, 0, sizeof(TrackedDevicePose_t));

	ovrPoseStatef ovrPose;

	if (index == k_unTrackedDeviceIndex_Hmd) {
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

	// If we haven't yet got a frame, mark the controller as having
	// an invalid pose to avoid errors from unnormalised 0,0,0,0 quaternions
	if (!ovrPose.TimeInSeconds) {
		pose->bPoseIsValid = false;
		return;
	}

	if (index == BaseSystem::leftHandIndex || index == BaseSystem::rightHandIndex) {
		static Posef transform = Posef(Quatf(BaseCompositor::GetHandTransform()), BaseCompositor::GetHandTransform().GetTranslation());

		ovrPose.ThePose = Posef(ovrPose.ThePose) * transform;
	}

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

void OculusBackend::GetSinglePose(
	ETrackingUniverseOrigin origin,
	TrackedDeviceIndex_t index,
	TrackedDevicePose_t* pose,
	ETrackingStateType trackingState) {

	if (trackingState == TrackingStateType_Now) {
		ovrTrackingState state = ovr_GetTrackingState(*ovr::session, 0 /* Most recent */, ovrTrue);
		GetSingleTrackingPose(origin, index, pose, state);
	}
	else {
		GetSingleTrackingPose(origin, index, pose, this->trackingState);
	}
}

void OculusBackend::GetDeviceToAbsoluteTrackingPose(
	vr::ETrackingUniverseOrigin toOrigin,
	float predictedSecondsToPhotonsFromNow,
	vr::TrackedDevicePose_t * poseArray,
	uint32_t poseArrayCount) {

	ovrTrackingState trackingState = { 0 };

	if (predictedSecondsToPhotonsFromNow == 0) {
		trackingState = ovr_GetTrackingState(*ovr::session, 0 /* Most recent */, ovrFalse);
	}
	else {
		trackingState = ovr_GetTrackingState(*ovr::session, ovr_GetTimeInSeconds() + predictedSecondsToPhotonsFromNow, ovrFalse);
	}

	for (uint32_t i = 0; i < poseArrayCount; i++) {
		GetSingleTrackingPose(toOrigin, i, &poseArray[i], trackingState);
	}

}
