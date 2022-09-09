#include "stdafx.h"
#define BASE_IMPL
#include "BaseExtendedDisplay.h"

#include <string>

// Unreal Engine uses this for some silly reason
void BaseExtendedDisplay::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	// Make up something that sounds about right
	*pnX = 0;
	*pnY = 0;
	*pnWidth = xr_main_view(XruEyeLeft).recommendedImageRectWidth * 2;
	*pnHeight = xr_main_view(XruEyeLeft).recommendedImageRectWidth;
}

void BaseExtendedDisplay::GetEyeOutputViewport(vr::EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	STUBBED();
}

void BaseExtendedDisplay::GetDXGIOutputInfo(int32_t* pnAdapterIndex, int32_t* pnAdapterOutputIndex)
{
	STUBBED();
}
