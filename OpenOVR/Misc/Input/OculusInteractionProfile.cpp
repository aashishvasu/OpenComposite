//
// Created by ZNix on 24/03/2021.
//

#include "stdafx.h"

#include "OculusInteractionProfile.h"

#include <glm/gtc/matrix_inverse.hpp>

OculusTouchInteractionProfile::OculusTouchInteractionProfile()
{

	const char* paths[] = {
		"/user/hand/left/input/x/click",
		"/user/hand/left/input/x/touch",
		"/user/hand/left/input/y/click",
		"/user/hand/left/input/y/touch",
		"/user/hand/left/input/menu/click",
		"/user/hand/right/input/a/click",
		"/user/hand/right/input/a/touch",
		"/user/hand/right/input/b/click",
		"/user/hand/right/input/b/touch",
		// Runtimes are not required to support the system button paths, and no OpenVR game can use it anyway.
		//"/user/hand/right/input/system/click",
	};

	const char* perHandPaths[] = {
		"input/squeeze/value",
		"input/trigger/value",
		"input/trigger/touch",
		"input/thumbstick/x",
		"input/thumbstick/y",
		"input/thumbstick/click",
		"input/thumbstick/touch",
		"input/thumbstick",
		"input/thumbrest/touch",
		"input/grip/pose",
		"input/aim/pose",
		"output/haptic",
	};

	for (const char* str : paths) {
		validInputPaths.insert(str);
	}

	for (const char* str : perHandPaths) {
		validInputPaths.insert("/user/hand/left/" + std::string(str));
		validInputPaths.insert("/user/hand/right/" + std::string(str));
	}

	pathTranslationMap = {
		{ "grip", "squeeze" },
		{ "joystick", "thumbstick" },
		{ "pull", "value" },
		{ "grip/click", "squeeze/value" },
		{ "trigger/click", "trigger/value" },
		{ "application_menu", "menu" }
	};
	// TODO implement the poses through the interaction profile (the raw pose is hard-coded in BaseInput at the moment):
	// pose/raw
	// pose/base
	// pose/handgrip
	// pose/tip

	hmdPropertiesMap = {
		{ vr::Prop_ManufacturerName_String, "Oculus" },
	};

	propertiesMap = {
		{ vr::Prop_ModelNumber_String, { "Oculus Quest2 (Left Controller)", "Oculus Quest2 (Right Controller)" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } }
	};

	// Setup the grip-to-steamvr space matrices

	// This is the matrices pulled from GetComponentState("handgrip") with SteamVR - they're
	// the opposite of what we want, since they transform from the SteamVR space to the grip space.
	// Both hands have the same matrix.
	glm::mat4 inverseHandTransform = {
		{ 1.000000, 0.000000, 0.000000, 0.000000 }, // Column 0
		{ 0.000000, 0.996138, 0.087799, 0.000000 }, // Column 1
		{ 0.000000, -0.087799, 0.996138, 0.000000 }, // Column 2
		{ 0.000000, 0.003000, 0.097000, 1.000000 }, // Column 3 (translation)
	};
	leftHandGripTransform = rightHandGripTransform = glm::affineInverse(inverseHandTransform);

	// Set up the component transforms
	leftComponentTransforms["base"] = {
		{ -1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.999976, 0.006981, 0.000000 },
		{ -0.000000, 0.006981, -0.999976, 0.000000 },
		{ -0.003400, -0.003400, 0.149100, 1.000000 },
	};
	rightComponentTransforms["base"] = {
		{ -1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.999976, 0.006981, 0.000000 },
		{ -0.000000, 0.006981, -0.999976, 0.000000 },
		{ 0.003400, -0.003400, 0.149100, 1.000000 },
	};
	leftComponentTransforms["tip"] = {
		{ 1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.794415, -0.607376, 0.000000 },
		{ 0.000000, 0.607376, 0.794415, 0.000000 },
		{ 0.016694, -0.025220, 0.024687, 1.000000 },
	};
	rightComponentTransforms["tip"] = {
		{ 1.000000, 0.000000, 0.000000, 0.000000 },
		{ 0.000000, 0.794415, -0.607376, 0.000000 },
		{ 0.000000, 0.607376, 0.794415, 0.000000 },
		{ -0.016694, -0.025220, 0.024687, 1.000000 },
	};
}

const std::string& OculusTouchInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/oculus/touch_controller";
	return path;
}

const InteractionProfile::LegacyBindings* OculusTouchInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings allBindings[2] = { {}, {} };
	int hand = handPath == "/user/hand/left" ? vr::Eye_Left : vr::Eye_Right;
	LegacyBindings& bindings = allBindings[hand];

	// First-time initialisation
	if (!bindings.menu) {
		bindings = {};
		bindings.stickX = "input/thumbstick/x";
		bindings.stickY = "input/thumbstick/y";
		bindings.stickBtn = "input/thumbstick/click";
		bindings.stickBtnTouch = "input/thumbstick/touch";

		bindings.trigger = "input/trigger/value";
		bindings.triggerClick = "input/trigger/value";
		bindings.triggerTouch = "input/trigger/touch";

		bindings.grip = "input/squeeze/value";

		bindings.haptic = "output/haptic";

		bindings.gripPoseAction = "input/grip/pose";
		bindings.aimPoseAction = "input/aim/pose";

		if (handPath == "/user/hand/left") {
			// Left
			bindings.menu = "input/y/click";
			bindings.menuTouch = "input/y/touch";
			bindings.btnA = "input/x/click";
			bindings.btnATouch = "input/x/touch";

			// Note this refers to what Oculus calls the menu button (and games use to open the pause menu), which
			// is used by SteamVR for it's menu.
			bindings.system = "input/menu/click";
		} else {
			// Right
			bindings.menu = "input/b/click";
			bindings.menuTouch = "input/b/touch";
			bindings.btnA = "input/a/click";
			bindings.btnATouch = "input/a/touch";

			// Ignore Oculus's system button, you're not supposed to do anything with it
		}
	}

	return &bindings;
}

std::optional<const char*> OculusTouchInteractionProfile::GetOpenVRName() const
{
	return "oculus_touch";
}
