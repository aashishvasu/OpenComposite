#pragma once

#include "OpenVR/interfaces/vrtypes.h"

// TODO remove this
#include "../OpenOVR/logging.h"

#include <string>

#define STUBBED() { \
	std::string str = "Hit stubbed file at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_ABORT(str.c_str()); \
}

/** Errors that can occur with the VR compositor */
enum OCOVR_EVRCompositorError {
	VRCompositorError_None = 0,
	VRCompositorError_RequestFailed = 1,
	VRCompositorError_IncompatibleVersion = 100,
	VRCompositorError_DoNotHaveFocus = 101,
	VRCompositorError_InvalidTexture = 102,
	VRCompositorError_IsNotSceneApplication = 103,
	VRCompositorError_TextureIsOnWrongDevice = 104,
	VRCompositorError_TextureUsesUnsupportedFormat = 105,
	VRCompositorError_SharedTexturesNotSupported = 106,
	VRCompositorError_IndexOutOfRange = 107,
	VRCompositorError_AlreadySubmitted = 108,
	VRCompositorError_InvalidBounds = 109,
};
