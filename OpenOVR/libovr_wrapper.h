#pragma once
#include "OVR_CAPI.h"

namespace ovr {
	extern ovrSession *session;
	extern ovrGraphicsLuid *luid;
	extern ovrHmdDesc hmdDesc;

	extern ovrEyeRenderDesc eyeRenderDesc[2];
	extern ovrPosef      hmdToEyeViewPose[2];

	extern int dxDeviceId;

	void Setup();
	bool IsAvailable();
	void Shutdown();
};
