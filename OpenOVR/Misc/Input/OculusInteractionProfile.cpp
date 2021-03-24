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
		validInputPaths.emplace_back(*str);
	}

	for (const char** str = perHandPaths; *str; str++) {
		validInputPaths.push_back("/user/hand/left/" + std::string(*str));
		validInputPaths.push_back("/user/hand/right/" + std::string(*str));
	}

	validInputPathsSet.insert(validInputPaths.begin(), validInputPaths.end());

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

const std::vector<VirtualInputFactory>& OculusTouchInteractionProfile::GetVirtualInputs() const
{
	return virtualInputs;
}
