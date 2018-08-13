#include "stdafx.h"
#define CVR_IMPL
#include "CVRSystem017.h"
CVR_GEN_IMPL(CVRSystem_017);

#define SIMPLE_REDIR(rettype, name) \
rettype CVRSystem_017::name() { \
	return base.name(); \
}

#define ADV_GETSET_REDIR(type, name, getter, setter) \
SIMPLE_REDIR(type, getter ## name) \
void CVRSystem_017::setter ## name(type val) { \
	return base.setter ## name(val); \
}

#define GETSET_REDIR(type, name) ADV_GETSET_REDIR(type, name, Get, Set)

using namespace std;

void CVRSystem_017::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	return base.GetRecommendedRenderTargetSize(width, height);
}

HmdMatrix44_t CVRSystem_017::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	return base.GetProjectionMatrix(eye, znear, zfar);
}

void CVRSystem_017::GetProjectionRaw(EVREye eEye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
	return base.GetProjectionRaw(eEye, pfLeft, pfRight, pfTop, pfBottom);
}

bool CVRSystem_017::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * pDistortionCoordinates) {
	return base.ComputeDistortion(eEye, fU, fV, pDistortionCoordinates);
}

HmdMatrix34_t CVRSystem_017::GetEyeToHeadTransform(EVREye eEye) {
	return base.GetEyeToHeadTransform(eEye);
}

bool CVRSystem_017::GetTimeSinceLastVsync(float * pfSecondsSinceLastVsync, uint64_t * pulFrameCounter) {
	return base.GetTimeSinceLastVsync(pfSecondsSinceLastVsync, pulFrameCounter);
}

SIMPLE_REDIR(int32_t, GetD3D9AdapterIndex);

void CVRSystem_017::GetDXGIOutputInfo(int32_t * adapterIndex) {
	return base.GetDXGIOutputInfo(adapterIndex);
}

SIMPLE_REDIR(bool, IsDisplayOnDesktop);

void CVRSystem_017::GetOutputDevice(uint64_t * pnDevice, ETextureType textureType, VkInstance_T * pInstance) {
	return base.GetOutputDevice(pnDevice, textureType, pInstance);
}

bool CVRSystem_017::SetDisplayVisibility(bool bIsVisibleOnDesktop) {
	return base.SetDisplayVisibility(bIsVisibleOnDesktop);
}

void CVRSystem_017::GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow,
	VR_ARRAY_COUNT(unTrackedDevicePoseArrayCount)TrackedDevicePose_t * pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) {

	return base.GetDeviceToAbsoluteTrackingPose(eOrigin, fPredictedSecondsToPhotonsFromNow, pTrackedDevicePoseArray, unTrackedDevicePoseArrayCount);
}

SIMPLE_REDIR(void, ResetSeatedZeroPose);
SIMPLE_REDIR(HmdMatrix34_t, GetSeatedZeroPoseToStandingAbsoluteTrackingPose);
SIMPLE_REDIR(HmdMatrix34_t, GetRawZeroPoseToStandingAbsoluteTrackingPose);

uint32_t CVRSystem_017::GetSortedTrackedDeviceIndicesOfClass(ETrackedDeviceClass eTrackedDeviceClass,
	VR_ARRAY_COUNT(unTrackedDeviceIndexArrayCount)vr::TrackedDeviceIndex_t * punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount,
	vr::TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex) {

	return base.GetSortedTrackedDeviceIndicesOfClass(eTrackedDeviceClass, punTrackedDeviceIndexArray,
		unTrackedDeviceIndexArrayCount, unRelativeToTrackedDeviceIndex);
}

EDeviceActivityLevel CVRSystem_017::GetTrackedDeviceActivityLevel(vr::TrackedDeviceIndex_t unDeviceId) {
	return base.GetTrackedDeviceActivityLevel(unDeviceId);
}

void CVRSystem_017::ApplyTransform(TrackedDevicePose_t * pOutputPose, const TrackedDevicePose_t * pTrackedDevicePose, const HmdMatrix34_t * pTransform) {
	return base.ApplyTransform(pOutputPose, pTrackedDevicePose, pTransform);
}

vr::TrackedDeviceIndex_t CVRSystem_017::GetTrackedDeviceIndexForControllerRole(vr::ETrackedControllerRole unDeviceType) {
	return base.GetTrackedDeviceIndexForControllerRole(unDeviceType);
}

vr::ETrackedControllerRole CVRSystem_017::GetControllerRoleForTrackedDeviceIndex(vr::TrackedDeviceIndex_t unDeviceIndex) {
	return base.GetControllerRoleForTrackedDeviceIndex(unDeviceIndex);
}

ETrackedDeviceClass CVRSystem_017::GetTrackedDeviceClass(vr::TrackedDeviceIndex_t deviceIndex) {
	return base.GetTrackedDeviceClass(deviceIndex);
}

bool CVRSystem_017::IsTrackedDeviceConnected(vr::TrackedDeviceIndex_t deviceIndex) {
	return base.IsTrackedDeviceConnected(deviceIndex);
}

bool CVRSystem_017::GetBoolTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	return base.GetBoolTrackedDeviceProperty(unDeviceIndex, prop, pErrorL);
}

float CVRSystem_017::GetFloatTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	return base.GetFloatTrackedDeviceProperty(unDeviceIndex, prop, pErrorL);
}

int32_t CVRSystem_017::GetInt32TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	return base.GetInt32TrackedDeviceProperty(unDeviceIndex, prop, pErrorL);
}

uint64_t CVRSystem_017::GetUint64TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	return base.GetUint64TrackedDeviceProperty(unDeviceIndex, prop, pErrorL);
}

HmdMatrix34_t CVRSystem_017::GetMatrix34TrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError * pErrorL) {
	return base.GetMatrix34TrackedDeviceProperty(unDeviceIndex, prop, pErrorL);
}

uint32_t CVRSystem_017::GetStringTrackedDeviceProperty(vr::TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop,
	VR_OUT_STRING() char * value, uint32_t bufferSize, ETrackedPropertyError * pErrorL) {

	return base.GetStringTrackedDeviceProperty(unDeviceIndex, prop, value, bufferSize, pErrorL);
}

const char * CVRSystem_017::GetPropErrorNameFromEnum(ETrackedPropertyError error) {
	return base.GetPropErrorNameFromEnum(error);
}

bool CVRSystem_017::PollNextEvent(VREvent_t * pEvent, uint32_t uncbVREvent) {
	return base.PollNextEvent(pEvent, uncbVREvent);
}

bool CVRSystem_017::PollNextEventWithPose(ETrackingUniverseOrigin eOrigin, VREvent_t * pEvent, uint32_t uncbVREvent, vr::TrackedDevicePose_t * pTrackedDevicePose) {
	return base.PollNextEventWithPose(eOrigin, pEvent, uncbVREvent, pTrackedDevicePose);
}

const char * CVRSystem_017::GetEventTypeNameFromEnum(EVREventType eType) {
	return base.GetEventTypeNameFromEnum(eType);
}

HiddenAreaMesh_t CVRSystem_017::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	return base.GetHiddenAreaMesh(eEye, type);
}

bool CVRSystem_017::GetControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex,
	vr::VRControllerState_t * controllerState, uint32_t controllerStateSize) {

	return base.GetControllerState(controllerDeviceIndex, controllerState, controllerStateSize);
}

bool CVRSystem_017::GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, vr::TrackedDeviceIndex_t unControllerDeviceIndex,
	vr::VRControllerState_t * pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t * pTrackedDevicePose) {

	return base.GetControllerStateWithPose(eOrigin, unControllerDeviceIndex, pControllerState, unControllerStateSize, pTrackedDevicePose);
}

void CVRSystem_017::TriggerHapticPulse(vr::TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec) {
	return base.TriggerHapticPulse(unControllerDeviceIndex, unAxisId, usDurationMicroSec);
}

const char * CVRSystem_017::GetButtonIdNameFromEnum(EVRButtonId eButtonId) {
	return base.GetButtonIdNameFromEnum(eButtonId);
}

const char * CVRSystem_017::GetControllerAxisTypeNameFromEnum(EVRControllerAxisType eAxisType) {
	return base.GetControllerAxisTypeNameFromEnum(eAxisType);
}

SIMPLE_REDIR(bool, CaptureInputFocus);
SIMPLE_REDIR(void, ReleaseInputFocus);
SIMPLE_REDIR(bool, IsInputFocusCapturedByAnotherProcess);

uint32_t CVRSystem_017::DriverDebugRequest(vr::TrackedDeviceIndex_t unDeviceIndex, const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize) {
	return base.DriverDebugRequest(unDeviceIndex, pchRequest, pchResponseBuffer, unResponseBufferSize);
}

vr::EVRFirmwareError CVRSystem_017::PerformFirmwareUpdate(vr::TrackedDeviceIndex_t unDeviceIndex) {
	return base.PerformFirmwareUpdate(unDeviceIndex);
}

SIMPLE_REDIR(void, AcknowledgeQuit_Exiting);
SIMPLE_REDIR(void, AcknowledgeQuit_UserPrompt);

#undef SIMPLE_REDIR
#undef ADV_GETSET_REDIR
#undef GETSET_REDIR
