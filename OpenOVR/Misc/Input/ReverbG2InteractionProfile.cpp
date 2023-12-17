#include "ReverbG2InteractionProfile.h"

ReverbG2InteractionProfile::ReverbG2InteractionProfile()
{
	std::string left_hand_paths[] = {
		"/input/x/click",
		"/input/y/click"
	};

	std::string right_hand_paths[] = {
		"/input/a/click",
		"/input/b/click"
	};

	std::string paths[] = {
		"/input/menu/click",
		"/input/squeeze/value",
		"/input/trigger/value",
		"/input/thumbstick",
		"/input/thumbstick/x",
		"/input/thumbstick/y",
		"/input/thumbstick/click",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic",
	};

	for (const auto& path : left_hand_paths) {
		validInputPaths.insert("/user/hand/left" + path);
	}

	for (const auto& path : right_hand_paths) {
		validInputPaths.insert("/user/hand/right" + path);
	}

	for (const auto& path : paths) {
		validInputPaths.insert("/user/hand/left" + path);
		validInputPaths.insert("/user/hand/right" + path);
	}

	pathTranslationMap = {
		{ "grip", "squeeze" },
		{ "joystick", "thumbstick" },
		{ "pull", "value" },
		{ "grip/click", "squeeze/value" },
		{ "trigger/click", "trigger/value" },
		{ "application_menu", "menu" }
	};

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "WindowsMR" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "WindowsMR" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } },
	};

	// Setup the grip-to-steamvr space matrices
	glm::mat4 inverseHandTransformLeft = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, -0.00553, 0.09689, 1.00000 }
	};

	glm::mat4 inverseHandTransformRight = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, -0.00553, 0.09689, 1.00000 }
	};

	leftHandGripTransform = glm::affineInverse(inverseHandTransformLeft);
	rightHandGripTransform = glm::affineInverse(inverseHandTransformRight);

	// Set up the component transforms

	glm::mat4 bodyLeft = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, -0.00553, 0.09689, 1.00000 }
	};
	glm::mat4 bodyRight = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, -0.00553, 0.09689, 1.00000 },
	};
	glm::mat4 tipLeft = {
		{ 1.00000, 0.00000, 0.00000, 0.00000 },
		{ -0.00000, 0.86635, 0.49944, 0.00000 },
		{ 0.00000, -0.49944, 0.86635, 0.00000 },
		{ -0.00068, -0.02634, 0.03009, 1.00000 }
	};
	glm::mat4 tipRight = {
		{ 1.00000, 0.00000, 0.00000, 0.00000 },
		{ -0.00000, 0.86635, 0.49944, 0.00000 },
		{ 0.00000, -0.49944, 0.86635, 0.00000 },
		{ 0.00068, -0.02634, 0.03009, 1.00000 }
	};

	leftComponentTransforms["body"] = glm::affineInverse(bodyLeft);
	rightComponentTransforms["body"] = glm::affineInverse(bodyRight);
	leftComponentTransforms["tip"] = glm::affineInverse(tipLeft);
	rightComponentTransforms["tip"] = glm::affineInverse(tipRight);
}

const std::string& ReverbG2InteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/hp/mixed_reality_controller";
	return path;
}

std::optional<const char*> ReverbG2InteractionProfile::GetLeftHandRenderModelName() const
{
	return std::nullopt; // fixme: fill proper model
}

std::optional<const char*> ReverbG2InteractionProfile::GetRightHandRenderModelName() const
{
	return std::nullopt; // fixme: fill proper model
}

std::optional<const char*> ReverbG2InteractionProfile::GetOpenVRName() const
{
	return "hpmotioncontroller";
}

const InteractionProfile::LegacyBindings* ReverbG2InteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings allBindings[2] = { {}, {} };
	int hand = handPath == "/user/hand/left" ? vr::Eye_Left : vr::Eye_Right;
	LegacyBindings& bindings = allBindings[hand];

	if (!bindings.menu) {
		bindings.system = "input/menu/click";
		bindings.stickX = "input/thumbstick/x";
		bindings.stickY = "input/thumbstick/y";
		bindings.stickBtn = "input/thumbstick/click";
		bindings.trigger = "input/trigger/value";
		bindings.triggerClick = "input/trigger/value";
		bindings.triggerTouch = "input/trigger/value";
		bindings.grip = "input/squeeze/value";
		bindings.haptic = "output/haptic";
		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";

		// TODO: figure out mappings for buttons on controllers
		if (handPath == "/user/hand/left") {
			bindings.btnA = "input/x/click";
			bindings.menu = "input/y/click";
		} else {
			bindings.btnA = "input/a/click";
			bindings.menu = "input/b/click";
		}
	}

	return &bindings;
}
