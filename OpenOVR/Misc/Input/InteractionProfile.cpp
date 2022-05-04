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

void InteractionProfile::AddLegacyBindings(const BaseInput::LegacyControllerActions& ctrl, std::vector<XrActionSuggestedBinding>& bindings) const
{
	auto create = [&](XrAction action, const char* path) {
		// Not supported on this controller?
		if (path == nullptr)
			return;

		// Bind the action to a suggested binding
		std::string realPath = ctrl.handPath + "/" + path;
		if (!IsInputPathValid(realPath)) {
			// No need for a soft abort, will only be called if an input profile gets it's paths wrong
			OOVR_ABORTF("Found legacy input path %s, not supported by profile", realPath.c_str());
		}

		// The action must have been defined, if not we'll get a harder-to-debug error from OpenXR later.
		OOVR_FALSE_ABORT(action != XR_NULL_HANDLE);

		XrActionSuggestedBinding binding = {};
		binding.action = action;

		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, realPath.c_str(), &binding.binding));

		bindings.push_back(binding);
	};

	const LegacyBindings* paths = GetLegacyBindings(ctrl.handPath);

	create(ctrl.system, paths->system);
	create(ctrl.menu, paths->menu);
	create(ctrl.menuTouch, paths->menuTouch);
	create(ctrl.btnA, paths->btnA);
	create(ctrl.btnATouch, paths->btnATouch);
	create(ctrl.stickX, paths->stickX);
	create(ctrl.stickY, paths->stickY);
	create(ctrl.stickBtn, paths->stickBtn);
	create(ctrl.stickBtnTouch, paths->stickBtnTouch);
	create(ctrl.trigger, paths->trigger);
	create(ctrl.triggerTouch, paths->triggerTouch);
	create(ctrl.grip, paths->grip);
	create(ctrl.haptic, paths->haptic);
	create(ctrl.gripPoseAction, paths->gripPoseAction);
	create(ctrl.aimPoseAction, paths->aimPoseAction);
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

void VirtualInput::OnPreFrame()
{
	// Make sure Update has been called this frame. This is necessary so that the
	// implementation class can calculate what changes on a frame-to-frame level.
	GetDigitalActionData(nullptr);
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

	// If null only do the update stuff above - this is used with OnPreFrame
	static_assert(sizeof(digital) == sizeof(*pActionData), "digital action data size mismatch");
	if (pActionData)
		memcpy(pActionData, &digital, sizeof(digital));

	return vr::VRInputError_None;
}

XrAction VirtualInput::CreateAction(const std::string& pathSuffix, XrActionType type, const std::string& localisedNameSuffix)
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
	actions.push_back(action);
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

const std::vector<XrAction>& VirtualInput::GetActionsForOriginLookup() const
{
	return actions;
}
