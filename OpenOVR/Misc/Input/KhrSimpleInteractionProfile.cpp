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
			validInputPaths.insert(std::string(*side) + "/" + std::string(*input));
		}
	}

	pathTranslationMap = {
		{ "application_menu", "menu" },
		{ "trigger/click", "select/click" },
	};
}

const std::string& KhrSimpleInteractionProfile::GetPath() const
{
	static std::string interactionPath = "/interaction_profiles/khr/simple_controller";
	return interactionPath;
}

std::optional<const char*> KhrSimpleInteractionProfile::GetLeftHandRenderModelName() const
{
	return std::nullopt;
}

std::optional<const char*> KhrSimpleInteractionProfile::GetRightHandRenderModelName() const
{
	return std::nullopt;
}

std::optional<const char*> KhrSimpleInteractionProfile::GetOpenVRName() const
{
	return std::nullopt;
}

const InteractionProfile::LegacyBindings* KhrSimpleInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings bindings = {};
	if (!bindings.menu) {
		// The stick, system button and grip are unsupported

		bindings.trigger = "input/select/click";
		bindings.triggerClick = "input/select/click";
		bindings.menu = "input/menu/click";

		bindings.haptic = "output/haptic";

		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";
	}
	return &bindings;
}
