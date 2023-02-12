#include "IndexControllerInteractionProfile.h"
#include "logging.h"

IndexControllerInteractionProfile::IndexControllerInteractionProfile()
{
	// Figured these out from OpenXR spec section 6.4
	std::string paths[] = {
		// Runtimes are not required to support the system button paths, and no OpenVR game can use them anyway.
		//"/input/system/click",
		//"/input/system/touch",
		"/input/a/click",
		"/input/a/touch",
		"/input/b/click",
		"/input/b/touch",
		"/input/squeeze/value",
		"/input/squeeze/force",
		"/input/trigger/click",
		"/input/trigger/value",
		"/input/trigger/touch",
		"/input/thumbstick/x",
		"/input/thumbstick/y",
		"/input/thumbstick/click",
		"/input/thumbstick/touch",
		"/input/thumbstick", // Allows directly binding 2 axis inputs like OpenVR does - see section 11.4 of the OpenXR spec
		"/input/trackpad/x",
		"/input/trackpad/y",
		"/input/trackpad/force",
		"/input/trackpad/touch",
		"/input/trackpad",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic"
	};

	for (const auto& path : paths) {
		this->validInputPaths.insert("/user/hand/left" + path);
		this->validInputPaths.insert("/user/hand/right" + path);
	}

	// These are substring replacements which translate from OpenVR paths to OpenXR paths.
	// I found the necessary examples of OpenVR paths in ./steamapps/common/NeosVR/Neos_Data/StreamingAssets/SteamVR/bindings_knuckles.json
	this->pathTranslationMap = {
		{ "thumbstick/position", "thumbstick" },
		{ "trackpad/position", "trackpad" },
		{ "trackpad/click", "trackpad/touch" },

		{ "pull", "value" },
		{ "application_menu", "system" },

		// All of these are "value" and not "force", because to activate "force" all the way
		// you have to pull really hard which is nerve racking
		// If you look at the bindings_knuckles.json you can see some more compliated thresholds, which I believe OC ignores.
		// If we add support for respecting those, change these to more reasonable replacements.
		{ "grip/force", "squeeze/value" },
		{ "grip/click", "squeeze/value" },
		{ "grip/touch", "squeeze/value" },
		{ "grip/value", "squeeze/value" },
	};

	//	this->bindingsLegacy.system = "input/system/click"; - causes issues on Oculus runtime
	this->bindingsLegacy.menu = "input/b/click";
	this->bindingsLegacy.btnA = "input/a/click";
	this->bindingsLegacy.btnATouch = "input/a/touch";

	this->bindingsLegacy.stickX = "input/thumbstick/x";
	this->bindingsLegacy.stickY = "input/thumbstick/y";
	this->bindingsLegacy.stickBtn = "input/thumbstick/click";
	this->bindingsLegacy.stickBtnTouch = "input/thumbstick/touch";
	this->bindingsLegacy.trigger = "input/trigger/click";
	this->bindingsLegacy.triggerTouch = "input/trigger/touch";

	this->bindingsLegacy.grip = "input/squeeze/value";
	this->bindingsLegacy.haptic = "output/haptic";

	this->bindingsLegacy.gripPoseAction = "input/grip/pose";
	this->bindingsLegacy.aimPoseAction = "input/aim/pose";

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "Valve" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "Knuckles Left", "Knuckles Right" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } }
	};
}

const std::string& IndexControllerInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/valve/index_controller";
	return path;
}

const InteractionProfile::LegacyBindings* IndexControllerInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	// Index controllers are exactly symmetrical, so we can just drop handPath on the floor.
	return &this->bindingsLegacy;
}

std::optional<const char*> IndexControllerInteractionProfile::GetOpenVRName() const
{
	return "knuckles";
}
