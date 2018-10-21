#include "stdafx.h"
#include "OCBaseSystem.h"

#include "libovr_wrapper.h"
#include "OVR_CAPI.h"

#include "ISystem_001.h"
using EVRExtendedButtonId = ocapi::IVROCSystem_001::EVRExtendedButtonId;

uint64_t OCBaseSystem::GetExtendedButtonStatus() {
	uint64_t val = 0;

	ovrInputState inputState;
	ovrResult result = ovr_GetInputState(*ovr::session, ovrControllerType_Touch, &inputState);
	if (!OVR_SUCCESS(result)) {
		std::string str = "[WARN] Could not get input: ";
		str += std::to_string(result);
		OOVR_LOG(str.c_str());
		return 0;
	}

	if (inputState.Buttons & ovrButton_Enter) {
		val |= vr::ButtonMaskFromId((vr::EVRButtonId) EVRExtendedButtonId::k_EButton_OVRMenu);
	}

	return val;
}
