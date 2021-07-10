#include "stdafx.h"
#include "libovr_wrapper.h"
#include "OVR_CAPI.h"

#define VALIDATE(x, msg) if (!OVR_SUCCESS(x)) { \
	char buff[1024]; \
	snprintf(buff, sizeof(buff), "%s: %d", (msg), x); \
	MessageBoxA(NULL, buff, "OpenComposite", MB_ICONERROR | MB_OK); \
	exit(-1); \
}

namespace ovr {
	ovrSession *session = NULL;
	ovrGraphicsLuid *luid = NULL;

	ovrHmdDesc hmdDesc;

	int dxDeviceId = -1;

	// Eye poses
	// TODO should these be updated every frame, incase the user adjusts their IPD?
	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrPosef      hmdToEyeViewPose[2];

	void Setup() {
		// Initializes LibOVR, and the Rift
		ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
		ovrResult result = ovr_Initialize(&initParams);
		VALIDATE(result, "Failed to initialize libOVR.");

		// Allocate the memory for the session and LUID
		session = new ovrSession();
		luid = new ovrGraphicsLuid();
		result = ovr_Create(session, luid);
		VALIDATE(result, "Failed to create libOVR session.");

		// Get the HMD information - we use this for FOV information, etc.
		// TODO run this every frame
		hmdDesc = ovr_GetHmdDesc(*session);

		// Fill out the eye pose stuff
		eyeRenderDesc[0] = ovr_GetRenderDesc(*session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(*session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);
		hmdToEyeViewPose[0] = eyeRenderDesc[0].HmdToEyePose;
		hmdToEyeViewPose[1] = eyeRenderDesc[1].HmdToEyePose;

		// It seems that SteamVR defaults to a floor-level tracking origin, while LibOVR does the opposite.
		ovr_SetTrackingOriginType(*session, ovrTrackingOrigin_FloorLevel);
	}

	bool IsAvailable() {
		ovrDetectResult result = ovr_Detect(0);

		// TODO if the Oculus service is not running, should we return false?
		return result.IsOculusHMDConnected;
	}

	void Shutdown() {
		ovr_Destroy(*session);
		ovr_Shutdown();

		session = NULL;
	}

	// TODO cleanup method
};
