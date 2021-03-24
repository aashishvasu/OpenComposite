//
// Created by ZNix on 27/02/2021.
//

#include "stdafx.h"

#include <utility>

#include "InteractionProfile.h"

#include "Reimpl/BaseInput.h"
#include "Reimpl/static_bases.gen.h"

const VirtualInputFactory* InteractionProfile::GetVirtualInput(const std::string& inputPath) const
{
	OOVR_FALSE_ABORT(donePostSetup);

	const auto& found = virtualInputNames.find(inputPath);
	if (found != virtualInputNames.end()) {
		return found->second;
	} else {
		return nullptr;
	}
}

void InteractionProfile::PostSetup()
{
	// Build the VirtualInput name-to-item mappings
	const std::vector<VirtualInputFactory>& inputs = GetVirtualInputs();
	for (const auto& input : inputs) {
		virtualInputNames[input.GetName()] = &input;
	}

	donePostSetup = true;
}

// VirtualInputFactory

VirtualInputFactory::VirtualInputFactory(std::string name, builder_t builder)
    : name(std::move(name)), builder(std::move(builder))
{
}

// VirtualInput

int VirtualInput::actionSerial;

VirtualInput::VirtualInput(BindInfo info)
    : bindInfo(std::move(info))
{
}

vr::EVRInputError VirtualInput::GetDigitalActionData(OOVR_InputDigitalActionData_t* pActionData)
{
	// Make sure we call update every time xrSyncActions is called, between those calls we
	// can safely cache the value.
	BaseInput* input = GetUnsafeBaseInput();
	if (digitalSerial != input->GetSyncSerial()) {
		Update();
		digitalSerial = input->GetSyncSerial();

		// TODO set activeOrigin
		if (digital.bActive) {
			OOVR_FALSE_ABORT(activeOrigin);
			digital.activeOrigin = activeOrigin;
		}
	}

	static_assert(sizeof(digital) == sizeof(*pActionData), "digital action data size mismatch");
	memcpy(pActionData, &digital, sizeof(digital));

	return vr::VRInputError_None;
}

XrAction VirtualInput::CreateAction(const std::string& pathSuffix, XrActionType type, const std::string& localisedNameSuffix) const
{
	XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };

	// Note: we need the serial in case the same action is bound to multiple controls with the same type of virtual input
	// This is not an elegant solution, but it works and we can clean it up later.
	std::string name = bindInfo.actionSetName + "-" + bindInfo.openvrActionName + "-" + std::to_string(actionSerial++) + "-" + pathSuffix;
	std::string localisedName = bindInfo.localisedName + " (" + std::to_string(actionSerial++) + ") " + localisedNameSuffix;

	info.actionType = type;
	strcpy_arr(info.actionName, name.c_str());
	strcpy_arr(info.localizedActionName, localisedName.c_str());

	// No subactions

	XrAction action = XR_NULL_HANDLE;
	OOVR_FAILED_XR_ABORT(xrCreateAction(bindInfo.actionSet, &info, &action));
	return action;
}

void VirtualInput::AddSuggestedBindings(std::vector<XrActionSuggestedBinding>& bindings)
{
	bindings.insert(bindings.end(), suggestedBindings.begin(), suggestedBindings.end());
}

void VirtualInput::PostInit()
{
	// At this point we should grab the activeOrigin

	// First find the string form of the first binding
	if (suggestedBindings.empty())
		OOVR_ABORTF("No suggested bindings for interaction profile %s/%s", bindInfo.actionSetName.c_str(), bindInfo.openvrActionName.c_str());
	XrPath firstXr = suggestedBindings.at(0).binding;
	uint32_t len;
	OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, firstXr, 0, &len, nullptr));
	std::vector<char> firstBuf(len);
	OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, firstXr, len, &len, firstBuf.data()));
	std::string first(firstBuf.begin(), firstBuf.end());

	// Now find the /user/hand/abc substring that it starts with
	int endOfHandPos = first.find('/', strlen("/user/hand/") + 1);
	first.erase(endOfHandPos);

	BaseInput* input = GetUnsafeBaseInput();
	activeOrigin = input->HandPathToIVH(first);
	OOVR_FALSE_ABORT(activeOrigin != vr::k_ulInvalidInputValueHandle);
}
