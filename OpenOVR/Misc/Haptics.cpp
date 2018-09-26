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
		OOVR_ABORT("invalid controller");
	}

	int numSamples = durationMicroseconds * 320.0f / 1000000.0f;

	if (numSamples > OVR_HAPTICS_BUFFER_SAMPLES_MAX)
		numSamples = OVR_HAPTICS_BUFFER_SAMPLES_MAX;
	else if (numSamples < 1)
		numSamples = 1;

	uint8_t *samples = new uint8_t[numSamples];

	// Generate the samples
	for (int i = 0; i < numSamples; i++) {
		// TODO should we use 160Hz?
		samples[i] = 255;
	}

	ovrHapticsBuffer buff;
	buff.SubmitMode = ovrHapticsBufferSubmit_Enqueue;
	buff.SamplesCount = numSamples;
	buff.Samples = samples;

	// TODO queue it up if the controller is busy
	// This should make the haptics work *much* better.
	ovr_SubmitControllerVibration(*ovr::session, ctrl, &buff);

	// Free up memory
	delete samples;
}
