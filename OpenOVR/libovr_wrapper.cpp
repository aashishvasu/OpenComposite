#include "stdafx.h"
#include "libovr_wrapper.h"
#include "OVR_CAPI.h"

#define VALIDATE(x, msg) if (!(x)) { MessageBoxA(NULL, (msg), "OculusRoomTiny", MB_ICONERROR | MB_OK); exit(-1); }

namespace ovr {
	ovrSession *session = NULL;
	ovrGraphicsLuid *luid = NULL;

	ovrHmdDesc hmdDesc;

	// Eye poses
	// TODO should these be updated every frame, incase the user adjusts their IPD?
	ovrEyeRenderDesc eyeRenderDesc[2];
	ovrPosef      hmdToEyeViewPose[2];

	void Setup() {
		// Initializes LibOVR, and the Rift
		ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
		ovrResult result = ovr_Initialize(&initParams);
		VALIDATE(OVR_SUCCESS(result), "Failed to initialize libOVR.");

		// Allocate the memory for the session and LUID
		session = new ovrSession();
		luid = new ovrGraphicsLuid();
		result = ovr_Create(session, luid);
		VALIDATE(OVR_SUCCESS(result), "Failed to create libOVR session.");

		// Get the HMD information - we use this for FOV information, etc.
		// TODO run this every frame
		hmdDesc = ovr_GetHmdDesc(*session);

		// Fill out the eye pose stuff
		eyeRenderDesc[0] = ovr_GetRenderDesc(*session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(*session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);
		hmdToEyeViewPose[0] = eyeRenderDesc[0].HmdToEyePose;
		hmdToEyeViewPose[1] = eyeRenderDesc[1].HmdToEyePose;
	}

	bool IsAvailable() {
		ovrDetectResult result = ovr_Detect(0);

		// TODO if the Oculus service is not running, should we return false?
		return result.IsOculusHMDConnected;
	}

	// TODO cleanup method
};
