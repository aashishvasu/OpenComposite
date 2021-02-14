#include "stdafx.h"
#define BASE_IMPL
#include "BaseInput.h"
#include <string>

#include "BaseClientCore.h"
#include "BaseSystem.h"
#include "Drivers/Backend.h"
#include "static_bases.gen.h"
#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <thread>
Json::Value _actionManifest;
Json::Value _bindingsJson;

using namespace std;
using namespace vr;

// This is a duplicate from BaseClientCore.cpp
static bool ReadJson(wstring path, Json::Value& result)
{
#ifndef _WIN32
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	ifstream in(converter.to_bytes(path), ios::binary);
#else
	ifstream in(path, ios::binary);
#endif
	if (in) {
		std::stringstream contents;
		contents << in.rdbuf();
		contents >> result;
		return true;
	} else {
		result = Json::Value(Json::ValueType::objectValue);
		return false;
	}
}

// Convert a UTF-8 string to a UTF-16 (wide) string
static std::wstring utf8to16(const std::string& t_str)
{
	//setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(t_str);
}

static std::string dirnameOf(const std::string& fname)
{
	size_t pos = fname.find_last_of("\\/");
	return (std::string::npos == pos)
	    ? ""
	    : fname.substr(0, pos);
}

// Case-insensitively compares two strings
static bool iequals(const string& a, const string& b)
{
	// from https://stackoverflow.com/a/4119881
	unsigned int sz = a.size();
	if (b.size() != sz)
		return false;
	for (unsigned int i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}

// ---

EVRInputError BaseInput::SetActionManifestPath(const char* pchActionManifestPath)
{
	STUBBED();
}
EVRInputError BaseInput::GetActionSetHandle(const char* pchActionSetName, VRActionSetHandle_t* pHandle)
{
	STUBBED();
}
EVRInputError BaseInput::GetActionHandle(const char* pchActionName, VRActionHandle_t* pHandle)
{
	STUBBED();
}
EVRInputError BaseInput::GetInputSourceHandle(const char* pchInputSourcePath, VRInputValueHandle_t* pHandle)
{
	STUBBED();
}

EVRInputError BaseInput::UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets,
    uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount)
{
	STUBBED();
}

EVRInputError BaseInput::GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
    InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}

EVRInputError BaseInput::GetPoseActionDataRelativeToNow(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	// Same function, different name - the 'RelativeToNow' suffix was added when GetPoseActionDataForNextFrame was added
	return GetPoseActionData(action, eOrigin, fPredictedSecondsFromNow, pActionData, unActionDataSize, ulRestrictToDevice);
}
EVRInputError BaseInput::GetPoseActionDataForNextFrame(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	return GetPoseActionData(action, eOrigin, 0, pActionData, unActionDataSize, ulRestrictToDevice);
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize)
{
	STUBBED();
}
EVRInputError BaseInput::GetDominantHand(vr::ETrackedControllerRole* peDominantHand)
{
	STUBBED();
}
EVRInputError BaseInput::SetDominantHand(vr::ETrackedControllerRole eDominantHand)
{
	STUBBED();
}
EVRInputError BaseInput::GetBoneCount(VRActionHandle_t action, uint32_t* pBoneCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetBoneHierarchy(VRActionHandle_t action, VR_ARRAY_COUNT(unIndexArayCount) BoneIndex_t* pParentIndices, uint32_t unIndexArayCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetBoneName(VRActionHandle_t action, BoneIndex_t nBoneIndex, VR_OUT_STRING() char* pchBoneName, uint32_t unNameBufferSize)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalReferenceTransforms(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalReferencePose eReferencePose, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalTrackingLevel(VRActionHandle_t action, EVRSkeletalTrackingLevel* pSkeletalTrackingLevel)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray,
    uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t action, EVRSummaryType eSummaryType, VRSkeletalSummaryData_t* pSkeletalSummaryData)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t action, VRSkeletalSummaryData_t* pSkeletalSummaryData)
{
	return GetSkeletalSummaryData(action, VRSummaryType_FromDevice, pSkeletalSummaryData);
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void* pvCompressedData, uint32_t unCompressedSize,
    uint32_t* punRequiredCompressedSize, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalMotionRange eMotionRange,
    VR_OUT_BUFFER_COUNT(unCompressedSize) void* pvCompressedData, uint32_t unCompressedSize, uint32_t* punRequiredCompressedSize)
{
	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(void* pvCompressedBuffer, uint32_t unCompressedBufferSize,
    EVRSkeletalTransformSpace* peTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray,
    uint32_t unTransformArrayCount)
{

	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(const void* pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace eTransformSpace,
    VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	STUBBED();
}

EVRInputError BaseInput::TriggerHapticVibrationAction(VRActionHandle_t action, float fStartSecondsFromNow, float fDurationSeconds,
    float fFrequency, float fAmplitude, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}

EVRInputError BaseInput::GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle,
    VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t* originsOut, uint32_t originOutCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize)
{
	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize,
    int32_t unStringSectionsToInclude)
{

	STUBBED();
}
EVRInputError BaseInput::GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t* pOriginInfo, uint32_t unOriginInfoSize)
{
	STUBBED();
}

/** Retrieves useful information about the bindings for an action */
EVRInputError BaseInput::GetActionBindingInfo(VRActionHandle_t actionHandle, OOVR_InputBindingInfo_t* pOriginInfo,
    uint32_t unBindingInfoSize, uint32_t unBindingInfoCount, uint32_t* punReturnedBindingInfoCount)
{

	STUBBED();
}

EVRInputError BaseInput::ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle)
{
	STUBBED();
}
EVRInputError BaseInput::ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets, uint32_t unSizeOfVRSelectedActionSet_t,
    uint32_t unSetCount, VRInputValueHandle_t originToHighlight)
{

	STUBBED();
}

EVRInputError BaseInput::GetComponentStateForBinding(const char* pchRenderModelName, const char* pchComponentName,
    const OOVR_InputBindingInfo_t* pOriginInfo, uint32_t unBindingInfoSize, uint32_t unBindingInfoCount,
    vr::RenderModel_ComponentState_t* pComponentState)
{
	STUBBED();
}

bool BaseInput::IsUsingLegacyInput()
{
	STUBBED();
}

// Interestingly enough this was added to IVRInput_007 without bumping the version number - that's fine since it's
// at the end of the vtable, but it's interesting that the version has always been bumped for this in the past.
EVRInputError BaseInput::OpenBindingUI(const char* pchAppKey, VRActionSetHandle_t ulActionSetHandle,
    VRInputValueHandle_t ulDeviceHandle, bool bShowOnDesktop)
{
	STUBBED();
}

EVRInputError BaseInput::GetBindingVariant(vr::VRInputValueHandle_t ulDevicePath, char* pchVariantArray, uint32_t unVariantArraySize)
{
	STUBBED();
}
