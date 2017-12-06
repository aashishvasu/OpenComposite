#include "stdafx.h"
#include "CVRSystem.h"
#include "OVR_CAPI.h"
#include "libovr_wrapper.h"
#include "ImplAccess.h"
#include "convert.h"

#include <string>

#define LEFT_HAND_DEVICE_INDEX 1
#define RIGHT_HAND_DEVICE_INDEX 2

using namespace std;

void CVRSystem::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	ovrSizei size = ovr_GetFovTextureSize(
		*ovr::session,
		ovrEye_Left, // Resolutions are done per-eye in LibOVR, no particular reason for left eye
		ovr::hmdDesc.DefaultEyeFov[ovrEye_Left],
		1.0f // 1.0x supersampling default, resulting in no stretched pixels (purpose of this function)
	);

	*width = size.w;
	*height = size.h;
}

HmdMatrix44_t CVRSystem::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	ovrMatrix4f matrix = ovrMatrix4f_Projection(
		ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)],
		znear, zfar,
		ovrProjection_None | ovrProjection_ClipRangeOpenGL // TODO API independent
	);

	return O2S_m4(matrix);
}

void CVRSystem::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
}

bool CVRSystem::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * pDistortionCoordinates) {
	return false;
}

HmdMatrix34_t CVRSystem::GetEyeToHeadTransform(EVREye ovr_eye) {
	ovrEyeType eye = S2O_eye(ovr_eye);
	ovrPosef &pose = ovr::hmdToEyeViewPose[eye];

	OVR::Matrix4f transform(pose);
	transform.Invert(); // Swap head->eye to eye->head

	HmdMatrix34_t result;
	O2S_om34(transform, result);
	return result;
}

bool CVRSystem::GetTimeSinceLastVsync(float * pfSecondsSinceLastVsync, uint64_t * pulFrameCounter) {
	throw "stub";
}

int32_t CVRSystem::GetD3D9AdapterIndex() {
	throw "stub";
}

void CVRSystem::GetDXGIOutputInfo(int32_t * pnAdapterIndex) {
	throw "stub";
}

void CVRSystem::GetOutputDevice(uint64_t * pnDevice, ETextureType textureType, VkInstance_T * pInstance) {
	throw "stub";
}

bool CVRSystem::IsDisplayOnDesktop() {
	return false; // Always in direct mode
}

bool CVRSystem::SetDisplayVisibility(bool bIsVisibleOnDesktop) {
	return false; // Always render in direct mode
}

void CVRSystem::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow,
	VR_ARRAY_COUNT(unTrackedDevicePoseArrayCount)TrackedDevicePose_t * pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {
	throw "stub";
}

void CVRSystem::ResetSeatedZeroPose() {
	throw "stub";
}

HmdMatrix34_t CVRSystem::GetSeatedZeroPoseToStandingAbsoluteTrackingPose() {
	throw "stub";
}

HmdMatrix34_t CVRSystem::GetRawZeroPoseToStandingAbsoluteTrackingPose() {
	throw "stub";
}

uint32_t CVRSystem::GetSortedTrackedDeviceIndicesOfClass(ETrackedDeviceClass eTrackedDeviceClass,
	VR_ARRAY_COUNT(unTrackedDeviceIndexArrayCount)vr::TrackedDeviceIndex_t * punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount,
	vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex) {
	throw "stub";
}

EDeviceActivityLevel CVRSystem::GetTrackedDeviceActivityLevel(vr::TrackedDeviceIndex_t unDeviceId) {
	throw "stub";
}

void CVRSystem::ApplyTransform(TrackedDevicePose_t * pOutputPose, const TrackedDevicePose_t * pTrackedDevicePose, const HmdMatrix34_t * pTransform) {
	throw "stub";
}

vr::TrackedDeviceIndex_t CVRSystem::GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole unDeviceType) {
	throw "stub";
}

vr::ETrackedControllerRole CVRSystem::GetControllerRoleForTrackedDeviceIndex(vr::TrackedDeviceIndex_t unDeviceIndex) {
	throw "stub";
}

ETrackedDeviceClass CVRSystem::GetTrackedDeviceClass(vr::TrackedDeviceIndex_t unDeviceIndex) {
	throw "stub";
}

bool CVRSystem::IsTrackedDeviceConnected(vr::TrackedDeviceIndex_t deviceIndex) {
	if (deviceIndex == k_unTrackedDeviceIndex_Hmd) {
		return true; // TODO
	}

	if (deviceIndex == LEFT_HAND_DEVICE_INDEX) {
		unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);
		return connected && ovrControllerType_LTouch != 0;
	} else if (deviceIndex == RIGHT_HAND_DEVICE_INDEX) {
		unsigned int connected = ovr_GetConnectedControllerTypes(*ovr::session);
		return connected && ovrControllerType_RTouch != 0;
	}

	return false;
}

bool CVRSystem::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

float CVRSystem::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

int32_t CVRSystem::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

uint64_t CVRSystem::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

HmdMatrix34_t CVRSystem::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	throw "stub";
}

uint32_t CVRSystem::GetStringTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop,
	VR_OUT_STRING() char * value, uint32_t bufferSize, ETrackedPropertyError * pErrorL) {

#define PROP(in, out) \
if(prop == in) { \
	if (value != NULL && bufferSize > 0) { \
		strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
	} \
	return strlen(out) + 1; \
}

	PROP(Prop_TrackingSystemName_String, "Constellation");
	PROP(Prop_SerialNumber_String, "<unknown>"); // TODO

#undef PROP

	return 0; // There are tonnes, and we're not implementing all of them.
}

const char * CVRSystem::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
	throw "stub";
}

bool CVRSystem::PollNextEvent(VREvent_t * pEvent, uint32_t uncbVREvent) {
	throw "stub";
}

bool CVRSystem::PollNextEventWithPose(ETrackingUniverseOrigin eOrigin, VREvent_t * pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t * pTrackedDevicePose) {
	throw "stub";
}

const char * CVRSystem::GetEventTypeNameFromEnum(EVREventType eType) {
	throw "stub";
}

HiddenAreaMesh_t CVRSystem::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	throw "stub";
}

bool CVRSystem::GetControllerState(vr::TrackedDeviceIndex_t unControllerDeviceIndex, vr::VRControllerState_t * pControllerState, uint32_t unControllerStateSize) {
	throw "stub";
}

bool CVRSystem::GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
	vr::VRControllerState_t * pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t * pTrackedDevicePose) {
	throw "stub";
}

void CVRSystem::TriggerHapticPulse(vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec) {
	throw "stub";
}

const char * CVRSystem::GetButtonIdNameFromEnum(EVRButtonId eButtonId) {
	throw "stub";
}

const char * CVRSystem::GetControllerAxisTypeNameFromEnum(EVRControllerAxisType eAxisType) {
	throw "stub";
}

bool CVRSystem::CaptureInputFocus() {
	throw "stub";
}

void CVRSystem::ReleaseInputFocus() {
	throw "stub";
}

bool CVRSystem::IsInputFocusCapturedByAnotherProcess() {
	throw "stub";
}

uint32_t CVRSystem::DriverDebugRequest(vr::TrackedDeviceIndex_t unDeviceIndex, const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) {
	throw "stub";
}

vr::EVRFirmwareError CVRSystem::PerformFirmwareUpdate(vr::TrackedDeviceIndex_t unDeviceIndex) {
	throw "stub";
}

void CVRSystem::AcknowledgeQuit_Exiting() {
	throw "stub";
}

void CVRSystem::AcknowledgeQuit_UserPrompt() {
	throw "stub";
}
