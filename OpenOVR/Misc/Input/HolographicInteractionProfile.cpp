#include "HolographicInteractionProfile.h"

HolographicInteractionProfile::HolographicInteractionProfile()
{
	std::string paths[] = {
		"/input/menu/click",
		"/input/squeeze/click",
		"/input/trigger/value",
		"/input/thumbstick/x",
		"/input/thumbstick/y",
		"/input/thumbstick/click",
		"/input/thumbstick",
		"/input/trackpad/x",
		"/input/trackpad/y",
		"/input/trackpad/click",
		"/input/trackpad/touch",
		"/input/trackpad",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic",
	};

	for (const auto& path : paths) {
		validInputPaths.insert("/user/hand/left" + path);
		validInputPaths.insert("/user/hand/right" + path);
	}

	pathTranslationMap = {
		{ "application_menu", "menu" },
		{ "grip", "squeeze" },
		{ "pull", "value" },
		{ "force", "value" },
		{ "trigger/click", "trigger/value" },
		{ "joystick", "thumbstick" }
	};

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "WindowsMR" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "WindowsMR" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } },
	};
}

const std::string& HolographicInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/microsoft/motion_controller";
	return path;
}

std::optional<const char*> HolographicInteractionProfile::GetOpenVRName() const
{
	return "holographic_controller";
}

const InteractionProfile::LegacyBindings* HolographicInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings bindings = {};

	// Games that use legacy bindings will typically just not use the thumbstick,
	// but most users would probably prefer to use it, so let's use it.
	if (!bindings.menu) {
		bindings.menu = "input/menu/click";
		bindings.stickX = "input/thumbstick/x";
		bindings.stickY = "input/thumbstick/y";
		bindings.stickBtn = "input/thumbstick/click";
		bindings.trigger = "input/trigger/value";
		bindings.grip = "input/squeeze/click";
		bindings.haptic = "output/haptic";
		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";
	}

	return &bindings;
}
