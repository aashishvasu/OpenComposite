//
// Created by ZNix on 24/03/2021.
//

#include "stdafx.h"

#include "OculusInteractionProfile.h"

#include "AnalogueToDigitalInput.h"

OculusTouchInteractionProfile::OculusTouchInteractionProfile()
{
	path = "/interaction_profiles/oculus/touch_controller";

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
		"/user/hand/right/input/system/click", // may not be available for application use
		nullptr
	};

	const char* perHandPaths[] = {
		"input/squeeze/value",
		"input/trigger/value",
		"input/trigger/touch",
		"input/thumbstick/x",
		"input/thumbstick/y",
		"input/thumbstick/click",
		"input/thumbstick/touch",
		"input/thumbrest/touch",
		"input/grip/pose",
		"input/aim/pose",
		"output/haptic",
		nullptr
	};

	for (const char** str = paths; *str; str++) {
		validInputPaths.insert(*str);
	}

	for (const char** str = perHandPaths; *str; str++) {
		validInputPaths.insert("/user/hand/left/" + std::string(*str));
		validInputPaths.insert("/user/hand/right/" + std::string(*str));
	}

	// Setup the virtual inputs
	virtualInputs.emplace_back(AnalogueToDigitalInput::Factory("/user/hand/left/input/trigger/value", "/user/hand/left/input/trigger/click"));
	virtualInputs.emplace_back(AnalogueToDigitalInput::Factory("/user/hand/right/input/trigger/value", "/user/hand/right/input/trigger/click"));
	virtualInputs.emplace_back(AnalogueToDigitalInput::Factory("/user/hand/left/input/squeeze/value", "/user/hand/left/input/grip/click"));
	virtualInputs.emplace_back(AnalogueToDigitalInput::Factory("/user/hand/right/input/squeeze/value", "/user/hand/right/input/grip/click"));
	// TODO add a way to remap the values for grip->squeeze, if an application does support handling that itself
	// TODO implement the following inputs:
	// joystick
	// system (remapped from menu)

	// TODO implement the poses through the interaction profile (the raw pose is hard-coded in BaseInput at the moment):
	// pose/raw
	// pose/base
	// pose/handgrip
	// pose/tip

	// TODO long-click versions of the buttons

	// Called last now that all our overridden functions will return their final values
	PostSetup();
}

const std::string& OculusTouchInteractionProfile::GetPath() const
{
	return path;
}

const std::unordered_set<std::string>& OculusTouchInteractionProfile::GetValidInputPaths() const
{
	return validInputPaths;
}

bool OculusTouchInteractionProfile::IsInputPathValid(const std::string& inputPath) const
{
	return validInputPaths.find(inputPath) != validInputPaths.end();
}

const std::vector<VirtualInputFactory>& OculusTouchInteractionProfile::GetVirtualInputs() const
{
	return virtualInputs;
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
