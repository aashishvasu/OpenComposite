//
// Created by ZNix on 24/03/2021.
//

#pragma once

#include "generated/interfaces/vrtypes.h"
struct OOVR_InputDigitalActionData_t {
	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;

	// The origin that caused this action's current state
	vr::VRInputValueHandle_t activeOrigin;

	// The current state of this action; will be true if currently pressed
	bool bState;

	// This is true if the state has changed since the last frame
	bool bChanged;

	// Time relative to now when this event happened. Will be negative to indicate a past time.
	float fUpdateTime;
};

struct OOVR_InputAnalogActionData_t {
	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;

	// The origin that caused this action's current state
	vr::VRInputValueHandle_t activeOrigin;

	// The current state of this action; will be delta updates for mouse actions
	float x, y, z;

	// Deltas since the previous call to UpdateActionState()
	float deltaX, deltaY, deltaZ;

	// Time relative to now when this event happened. Will be negative to indicate a past time.
	float fUpdateTime;
};

struct OOVR_InputPoseActionData_t {
	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;

	// The origin that caused this action's current state
	vr::VRInputValueHandle_t activeOrigin;

	// The current state of this action
	vr::TrackedDevicePose_t pose;
};
