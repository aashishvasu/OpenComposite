#include "ViveInteractionProfile.h"

ViveWandInteractionProfile::ViveWandInteractionProfile()
{
	std::string paths[] = {
		"/input/system/click",
		"/input/squeeze/click",
		"/input/menu/click",
		"/input/trigger/click",
		"/input/trigger/value",
		"/input/trackpad/x",
		"/input/trackpad/y",
		"/input/trackpad/click",
		"/input/trackpad/touch",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic"
	};

	for (const auto& path : paths) {
		validInputPaths.insert("/user/hand/left" + path);
		validInputPaths.insert("/user/hand/right" + path);
	}

	PostSetup();
}

const std::string& ViveWandInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/htc/vive_controller";
	return path;
}

const std::unordered_set<std::string>& ViveWandInteractionProfile::GetValidInputPaths() const
{
	return validInputPaths;
}

bool ViveWandInteractionProfile::IsInputPathValid(const std::string& inputPath) const
{
	return validInputPaths.find(inputPath) != validInputPaths.end();
}

const std::vector<VirtualInputFactory>& ViveWandInteractionProfile::GetVirtualInputs() const
{
	return virtualInputs;
}

const InteractionProfile::LegacyBindings* ViveWandInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings bindings = {};

	if (!bindings.menu) {
		bindings.system = "input/system/click";
		bindings.menu = "input/menu/click";
		bindings.stickX = "input/trackpad/x";
		bindings.stickY = "input/trackpad/y";
		bindings.stickBtn = "input/trackpad/click";
		bindings.stickBtnTouch = "input/trackpad/touch";
		bindings.trigger = "input/trigger/click";
		bindings.grip = "input/squeeze/click";
		bindings.haptic = "output/haptic";
		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";
	}

	return &bindings;
}

static std::map<std::string, std::string> translation_map = {
	{ "application_menu", "menu" },
};

std::string ViveWandInteractionProfile::TranslateAction(const std::string& inputPath) const
{
	if (inputPath.find("/user/hand") != std::string::npos && !IsInputPathValid(inputPath)) {
		for (const auto& [key, val] : translation_map) {
			size_t loc = inputPath.find(key);
			if (loc != std::string::npos) {
				std::string ret = inputPath.substr(0, loc) + val + inputPath.substr(loc + key.size());
				OOVR_LOGF("Translated path '%s' to '%s' for profile %s", inputPath.c_str(), ret.c_str(), GetPath().c_str());
				return ret;
			}
		}
	}
	return inputPath;
}
