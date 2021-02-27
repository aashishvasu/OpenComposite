//
// Created by ZNix on 27/02/2021.
//

#include "stdafx.h"

#include "InteractionProfile.h"

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
		validInputPaths.push_back(*str);
	}

	for (const char** str = perHandPaths; *str; str++) {
		validInputPaths.push_back("/user/hand/left/" + std::string(*str));
		validInputPaths.push_back("/user/hand/right/" + std::string(*str));
	}

	validInputPathsSet.insert(validInputPaths.begin(), validInputPaths.end());
}
