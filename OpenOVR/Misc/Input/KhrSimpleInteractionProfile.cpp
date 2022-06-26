//
// Created by znix on 17/04/2022.
//

#include "stdafx.h"

#include "KhrSimpleInteractionProfile.h"

KhrSimpleInteractionProfile::KhrSimpleInteractionProfile()
{
	const char* sides[] = {
		"/user/hand/left",
		"/user/hand/right",
		nullptr
	};
	const char* inputs[] = {
		"input/select/click",
		"input/menu/click",
		"input/grip/pose",
		"input/aim/pose",
		"output/haptic",
		nullptr
	};

	for (const char** side = sides; *side; side++) {
		for (const char** input = inputs; *input; input++) {
			validPaths.insert(std::string(*side) + "/" + std::string(*input));
		}
	}

	PostSetup();
}

const std::string& KhrSimpleInteractionProfile::GetPath() const
{
	static std::string interactionPath = "/interaction_profiles/khr/simple_controller";
	return interactionPath;
}

const std::unordered_set<std::string>& KhrSimpleInteractionProfile::GetValidInputPaths() const
{
	return validPaths;
}

bool KhrSimpleInteractionProfile::IsInputPathValid(const std::string& inputPath) const
{
	return validPaths.find(inputPath) != validPaths.end();
}

const std::vector<VirtualInputFactory>& KhrSimpleInteractionProfile::GetVirtualInputs() const
{
	return virtualInputs;
}

const InteractionProfile::LegacyBindings* KhrSimpleInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings bindings = {};
	if (!bindings.menu) {
		// The stick, system button and grip are unsupported

		bindings.trigger = "input/select/click";
		bindings.menu = "input/menu/click";

		bindings.haptic = "output/haptic";

		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";
	}
	return &bindings;
}

static const std::map<std::string, std::string> pathTranslationMap = {
	{ "application_menu", "menu" },
	{ "trigger", "select" },
};

std::string KhrSimpleInteractionProfile::TranslateAction(const std::string& inputPath) const
{
	// check if we have an invalid hand path
	if (!IsInputPathValid(inputPath) && inputPath.find("/user/hand/") != std::string::npos) {
		// try translating path
		for (auto& [key, val] : pathTranslationMap) {
			size_t loc = inputPath.find(key);
			if (loc != std::string::npos) {
				// translate action!
				std::string ret = inputPath.substr(0, loc) + val + inputPath.substr(loc + key.size());
				OOVR_LOGF("Translated path %s to %s for profile %s", inputPath.c_str(), ret.c_str(), GetPath().c_str());
				return ret;
			}
		}
	}

	// either this path is already valid or it's invalid and not translatable
	return inputPath;
}
