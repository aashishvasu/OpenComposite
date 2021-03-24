//
// Created by ZNix on 24/03/2021.
//

#include "stdafx.h"

#include "AnalogueToDigitalInput.h"

#include <utility>

VirtualInputFactory AnalogueToDigitalInput::Factory(const std::string& src, const std::string& name)
{
	return VirtualInputFactory(name, [src](const BindInfo& info) {
		return std::make_unique<AnalogueToDigitalInput>(info, src);
	});
}

AnalogueToDigitalInput::AnalogueToDigitalInput(BindInfo info, std::string src)
    : VirtualInput(std::move(info)), src(std::move(src))
{
	action = CreateAction("-a2d", XR_ACTION_TYPE_FLOAT_INPUT, "(Analogue-to-Digital)");

	XrPath srcPath;
	OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, this->src.c_str(), &srcPath));
	suggestedBindings.push_back({ action, srcPath });
}

void AnalogueToDigitalInput::Update()
{
	ZeroMemory(&digital, sizeof(digital));
	// TODO fUpdateTime

	XrActionStateGetInfo info = { XR_TYPE_ACTION_STATE_GET_INFO };
	info.action = action;
	XrActionStateFloat state = { XR_TYPE_ACTION_STATE_FLOAT };
	OOVR_FAILED_XR_ABORT(xrGetActionStateFloat(xr_session, &info, &state));

	// If our source isn't active, leave the digital data in it's inactive all-zero state
	if (!state.isActive) {
		active = false;
		return;
	}

	digital.bActive = true;
	// Ignore activeOrigin, it will eventually be set in InteractionProfile

	// Implement hysteresis
	if (state.currentState < 0.35)
		active = false;
	else if (state.currentState > 0.65)
		active = true;

	// TODO bChanged

	digital.bState = active;
}
