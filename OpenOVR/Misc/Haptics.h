#pragma once
#include <openxr/openxr.h>

class Haptics {
public:
	Haptics();
	~Haptics();

	void StartSimplePulse(XrAction action, unsigned int durationMicroseconds);

private:
	bool leftRunning, rightRunning;
};
