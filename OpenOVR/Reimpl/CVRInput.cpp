#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("Input", "004")
GEN_INTERFACE("Input", "005")
GEN_INTERFACE("Input", "006")
GEN_INTERFACE("Input", "007")
// version 8 and 9 are skipped
GEN_INTERFACE("Input", "010")

#include "generated/GVRInput.gen.h"

// Prior to IVRInput_005 / OpenVR 1.1.3 the input layer had a slightly different format, so adapt that
vr::EVRInputError CVRInput_004::GetSkeletalActionData(vr::VRActionHandle_t action, vr::IVRInput_004::InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize, vr::VRInputValueHandle_t ulRestrictToDevice)
{
	// Set the size to what the main method is expecting
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(vr::IVRInput_004::InputSkeletalActionData_t));
	unActionDataSize = sizeof(OOVR_InputSkeletalActionData_t);

	// Load the required value
	OOVR_FALSE_ABORT(base->GetBoneCount(action, &pActionData->boneCount) == vr::VRInputError_None);

	return base->GetSkeletalActionData(action, (OOVR_InputSkeletalActionData_t*)pActionData, unActionDataSize, ulRestrictToDevice);
}
