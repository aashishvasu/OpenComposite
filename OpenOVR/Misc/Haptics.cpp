#include "stdafx.h"

#include "Haptics.h"

Haptics::Haptics()
{
}

Haptics::~Haptics()
{
}

void Haptics::StartSimplePulse(XrAction action, unsigned int durationMicroseconds)
{
	XrHapticActionInfo info = {};
	info.type = XR_TYPE_HAPTIC_ACTION_INFO;
	info.action = action;

	XrHapticVibration haptic = {};
	haptic.type = XR_TYPE_HAPTIC_VIBRATION;
	haptic.duration = durationMicroseconds * 1000; // duration is in nanoseconds
	haptic.amplitude = 1;
	haptic.frequency = XR_FREQUENCY_UNSPECIFIED;
	XrResult result = xrApplyHapticFeedback(xr_session, &info, (const XrHapticBaseHeader*)&haptic);
	// TODO error checking

	OOVR_ABORT("Haptics not yet implemented"); // TODO check everything works
}
