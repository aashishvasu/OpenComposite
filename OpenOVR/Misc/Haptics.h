#pragma once
#include <OVR_CAPI.h>

class Haptics {
public:
	Haptics();
	~Haptics();

	void StartSimplePulse(ovrControllerType ctrl, unsigned int durationMicroseconds);

private:
	bool leftRunning, rightRunning;
};
