#include "stdafx.h"
#include "Haptics.h"
#include "libovr_wrapper.h"
#include <thread>
#include <chrono>
#include <future>

Haptics::Haptics() {
}

Haptics::~Haptics() {
}

void Haptics::StartSimplePulse(ovrControllerType ctrl, unsigned int durationMicroseconds) {
	bool left;
	if (ctrl == ovrControllerType_LTouch) {
		left = true;
	}
	else if (ctrl == ovrControllerType_RTouch) {
		left = false;
	}
	else {
		// TODO
		return;
	}

	/*
	// TODO amplitude and frequency settings
	ovr_SetControllerVibration(*ovr::session, ctrl, 0.5, 0.5);

	// Use async to launch a function (lambda) in parallel
	std::async(std::launch::async, [ctrl, durationMicroseconds]() {
		std::this_thread::sleep_for(std::chrono::microseconds(durationMicroseconds));

		ovr_SetControllerVibration(*ovr::session, ctrl, 0.5, 0.5);
	});
	*/
}
