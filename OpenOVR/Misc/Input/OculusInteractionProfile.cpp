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

	// New Data directly from the openxr-grip space for quest 2 controllers in steamvr
	// SteamVR\resources\rendermodels\oculus_quest2_controller_left
	/*
	    "openxr_grip" : {
	        "component_local":
	        {
	        "origin" : [ -0.007, -0.00182941, 0.1019482 ],
	                   "rotate_xyz" : [ 20.6, 0.0, 0.0 ]
	        }
	    }
	*/

	// Setup the grip-to-steamvr space matrices

	// Made from values in: SteamVR\resources\rendermodels\oculus_quest2_controller_left\oculus_quest2_controller_left.json
	glm::mat4 inverseHandTransformLeft = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, 0.00300, 0.09700, 1.00000 }
	};

	// Made from values in: SteamVR\resources\rendermodels\oculus_quest2_controller_right\oculus_quest2_controller_right.json
	glm::mat4 inverseHandTransformRight = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.99614, -0.08780, 0.00000 },
		{ 0.00000, 0.08780, 0.99614, 0.00000 },
		{ 0.00000, 0.00300, 0.09700, 1.00000 }
	};

	leftHandGripTransform = glm::affineInverse(inverseHandTransformLeft);
	rightHandGripTransform = glm::affineInverse(inverseHandTransformRight);

	// Set up the component transforms
	leftComponentTransforms["base"] = {
		{ -1.00000, 0.00000, -0.00000, 0.00000 },
		{ 0.00000, 0.99998, -0.00698, 0.00000 },
		{ 0.00000, -0.00698, -0.99998, 0.00000 },
		{ -0.00340, -0.00340, 0.14910, 1.00000 }
	};
	rightComponentTransforms["base"] = {
		{ -1.00000, 0.00000, -0.00000, 0.00000 },
		{ 0.00000, 0.99998, -0.00698, 0.00000 },
		{ 0.00000, -0.00698, -0.99998, 0.00000 },
		{ 0.00340, -0.00340, 0.14910, 1.00000 }
	};
	leftComponentTransforms["tip"] = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.79441, 0.60738, 0.00000 },
		{ -0.00000, -0.60738, 0.79441, 0.00000 },
		{ 0.01669, -0.02522, 0.02469, 1.00000 }
	};
	rightComponentTransforms["tip"] = {
		{ 1.00000, -0.00000, 0.00000, 0.00000 },
		{ 0.00000, 0.79441, 0.60738, 0.00000 },
		{ -0.00000, -0.60738, 0.79441, 0.00000 },
		{ -0.01669, -0.02522, 0.02469, 1.00000 }
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
