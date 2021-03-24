//
// Created by ZNix on 24/03/2021.
//

#pragma once

#include "InteractionProfile.h"

class AnalogueToDigitalInput : public VirtualInput {
public:
	AnalogueToDigitalInput(BindInfo info, std::string src);

	static VirtualInputFactory Factory(const std::string& src, const std::string& name);

protected:
	void Update() override;

private:
	std::string src;
	XrAction action = XR_NULL_HANDLE;

	/**
	 * Whether the input is currently on or not, used for implementing hysteresis.
	 */
	bool active = false;

	/**
	 * Used for calculating bChanged
	 */
	bool lastActive = false;
};
