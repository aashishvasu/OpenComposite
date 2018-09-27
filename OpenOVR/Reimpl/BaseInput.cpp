#include "stdafx.h"
#define BASE_IMPL
#include "BaseInput.h"
#include <string>

using namespace std;
using namespace vr;

EVRInputError BaseInput::SetActionManifestPath(const char *pchActionManifestPath) {
	STUBBED();
}
EVRInputError BaseInput::GetActionSetHandle(const char *pchActionSetName, VRActionSetHandle_t *pHandle) {
	STUBBED();
}
EVRInputError BaseInput::GetActionHandle(const char *pchActionName, VRActionHandle_t *pHandle) {
	STUBBED();
}
EVRInputError BaseInput::GetInputSourceHandle(const char *pchInputSourcePath, VRInputValueHandle_t  *pHandle) {
	STUBBED();
}
EVRInputError BaseInput::UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets,
	uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount) {

	STUBBED();
}
EVRInputError BaseInput::GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
	InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
	EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray,
	uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
	EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void *pvCompressedData, uint32_t unCompressedSize,
	uint32_t *punRequiredCompressedSize, VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(void *pvCompressedBuffer, uint32_t unCompressedBufferSize,
	EVRSkeletalTransformSpace *peTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray,
	uint32_t unTransformArrayCount) {

	STUBBED();
}
EVRInputError BaseInput::TriggerHapticVibrationAction(VRActionHandle_t action, float fStartSecondsFromNow, float fDurationSeconds,
	float fFrequency, float fAmplitude, VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle,
	VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t *originsOut, uint32_t originOutCount) {

	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize) {
	STUBBED();
}
EVRInputError BaseInput::GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t *pOriginInfo, uint32_t unOriginInfoSize) {
	STUBBED();
}
EVRInputError BaseInput::ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle) {
	STUBBED();
}
EVRInputError BaseInput::ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets, uint32_t unSizeOfVRSelectedActionSet_t,
	uint32_t unSetCount, VRInputValueHandle_t originToHighlight) {

	STUBBED();
}
