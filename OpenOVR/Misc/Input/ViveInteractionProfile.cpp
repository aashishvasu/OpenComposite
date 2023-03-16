#include "ViveInteractionProfile.h"

ViveWandInteractionProfile::ViveWandInteractionProfile()
{
	std::string paths[] = {
		// Runtimes are not required to support the system button paths, and no OpenVR game can use them anyway.
		//"/input/system/click",
		"/input/squeeze/click",
		"/input/menu/click",
		"/input/trigger/click",
		"/input/trigger/value",
		"/input/trackpad/x",
		"/input/trackpad/y",
		"/input/trackpad/click",
		"/input/trackpad/touch",
		"/input/trackpad",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic"
	};

	for (const auto& path : paths) {
		validInputPaths.insert("/user/hand/left" + path);
		validInputPaths.insert("/user/hand/right" + path);
	}

	pathTranslationMap = {
		{ "application_menu", "menu" },
		{ "grip/click", "squeeze/click" },
		{ "grip/value", "squeeze/click" },
		{ "pull", "value" }
	};

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "HTC" },
		{ vr::Prop_ModelNumber_String, "Vive. MV" },
		{ vr::Prop_ControllerType_String, "vive" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "Vive. Controller MV" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } },
	};
}

const std::string& ViveWandInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/htc/vive_controller";
	return path;
}

const InteractionProfile::LegacyBindings* ViveWandInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings bindings = {};

	if (!bindings.menu) {
		//		bindings.system = "input/system/click"; - causes issues on Oculus runtime
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

std::optional<const char*> ViveWandInteractionProfile::GetOpenVRName() const
{
	return "vive_controller";
}
