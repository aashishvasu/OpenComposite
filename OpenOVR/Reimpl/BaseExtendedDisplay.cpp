#include "stdafx.h"
#define BASE_IMPL
#include "BaseExtendedDisplay.h"

#include "libovr_wrapper.h"
#include "OVR_CAPI.h"
#include <string>

// Unreal Engine uses this for some silly reason
void BaseExtendedDisplay::GetWindowBounds(int32_t * pnX, int32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) {
	ovrSizei size = ovr_GetFovTextureSize(
		*ovr::session,
		ovrEye_Left, // Resolutions are done per-eye in LibOVR, no particular reason for left eye
		ovr::hmdDesc.DefaultEyeFov[ovrEye_Left],
		1.0f // 1.0x supersampling default, resulting in no stretched pixels (purpose of this function)
	);

	// Make up something that sounds about right
	*pnX = 0;
	*pnY = 0;
	*pnWidth = size.w * 2;
	*pnHeight = size.h;
}

void BaseExtendedDisplay::GetEyeOutputViewport(vr::EVREye eEye, uint32_t * pnX, uint32_t * pnY, uint32_t * pnWidth, uint32_t * pnHeight) {
	STUBBED();
}

void BaseExtendedDisplay::GetDXGIOutputInfo(int32_t * pnAdapterIndex, int32_t * pnAdapterOutputIndex) {
	STUBBED();
}
