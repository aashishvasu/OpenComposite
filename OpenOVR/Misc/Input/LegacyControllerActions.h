#pragma once

#include <openxr/openxr.h>
#include <string>

#include "Drivers/Backend.h"

struct LegacyControllerActions {

	std::string handPath; // eg /user/hand/left
	XrPath handPathXr;
	ITrackedDevice::TrackedDeviceType handType;

	// Matches up with EVRButtonId
	XrAction system; // 'system' button, on Vive the SteamVR buttons, on Oculus Touch the menu button on the left controller
	XrAction menu, menuTouch; // Upper button on touch controller (B/Y), application button on Vive
	XrAction btnA, btnATouch; // Lower button on touch controller - A/X, not present on Vive

	XrAction trackpadX, trackpadY, trackpadTouch, trackpadForce, stickX, stickY, stickBtn, stickBtnTouch; // Axis0

	// For the trigger and grip, we use separate actions for digital and analogue input. If the physical input is analogue it
	// saves us from having to implement hysteresis (and the runtime probably knows what the appropriate thresholds are better
	// than we do) and generally gives more flexibility on exotic hardware, as the user can rebind them separately.
	XrAction trigger, triggerClick, triggerTouch; // Axis1
	XrAction grip, gripClick; // Axis2

	XrAction haptic;

	// Note: the 'grip' pose is like the natural holding pose of a controller, the 'aim' pose
	// is a (runtime dependent) way for essentially using a controller like a pointer.
	// Note: The skeletal input functions all work inside the grip space, not sure if SteamVR does it this way.
	XrAction gripPoseAction, aimPoseAction;
	XrSpace gripPoseSpace, aimPoseSpace;
};
