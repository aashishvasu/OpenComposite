//
// Created by ZNix on 27/02/2021.
//

#include "stdafx.h"

#include <utility>

#include "HolographicInteractionProfile.h"
#include "IndexControllerInteractionProfile.h"
#include "InteractionProfile.h"
#include "KhrSimpleInteractionProfile.h"
#include "OculusInteractionProfile.h"
#include "ReverbG2InteractionProfile.h"
#include "ViveInteractionProfile.h"

#include "Reimpl/BaseInput.h"
#include "generated/static_bases.gen.h"

std::string InteractionProfile::TranslateAction(const std::string& inputPath) const
{
	if (!pathTranslationMap.empty() && !IsInputPathValid(inputPath)) {
		std::string ret = inputPath;
		for (auto& [key, val] : pathTranslationMap) {
			size_t loc = ret.find(key);
			if (loc != std::string::npos) {
				// translate action!
				ret = ret.substr(0, loc) + val + ret.substr(loc + key.size());
			}
		}
		OOVR_LOGF("Translated path %s to %s for profile %s", inputPath.c_str(), ret.c_str(), GetPath().c_str());
		return ret;
	}
	// either this path is already valid or it's invalid and not translatable
	return inputPath;
}

const std::unordered_set<std::string>& InteractionProfile::GetValidInputPaths() const
{
	return validInputPaths;
}

bool InteractionProfile::IsInputPathValid(const std::string& inputPath) const
{
	return validInputPaths.find(inputPath) != validInputPaths.end();
}

void InteractionProfile::AddLegacyBindings(const LegacyControllerActions& ctrl, std::vector<XrActionSuggestedBinding>& bindings) const
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
	create(ctrl.triggerClick, paths->trigger);
	create(ctrl.grip, paths->grip);
	create(ctrl.gripClick, paths->grip);
	create(ctrl.haptic, paths->haptic);
	create(ctrl.gripPoseAction, paths->gripPoseAction);
	create(ctrl.aimPoseAction, paths->aimPoseAction);
}

const InteractionProfile::ProfileList& InteractionProfile::GetProfileList()
{
	static std::vector<std::unique_ptr<InteractionProfile>> profiles;
	if (profiles.empty()) {
		if (xr_ext->G2Controller_Available())
			profiles.emplace_back(std::make_unique<ReverbG2InteractionProfile>());

		profiles.emplace_back(std::make_unique<HolographicInteractionProfile>());
		profiles.emplace_back(std::make_unique<IndexControllerInteractionProfile>());
		profiles.emplace_back(std::make_unique<ViveWandInteractionProfile>());
		profiles.emplace_back(std::make_unique<OculusTouchInteractionProfile>());
		profiles.emplace_back(std::make_unique<KhrSimpleInteractionProfile>());
	}
	return profiles;
}

InteractionProfile* InteractionProfile::GetProfileByPath(const string& name)
{
	std::map<std::string, InteractionProfile*> byPath;
	if (byPath.empty()) {
		for (const std::unique_ptr<InteractionProfile>& profile : GetProfileList()) {
			byPath[profile->GetPath()] = profile.get();
		}
	}
	if (!byPath.contains(name))
		OOVR_ABORTF("Could not find interaction profile '%s'", name.c_str());
	return byPath.at(name);
}

glm::mat4 InteractionProfile::GetGripToSteamVRTransform(ITrackedDevice::HandType hand) const
{
	return glm::identity<glm::mat4>();
}
