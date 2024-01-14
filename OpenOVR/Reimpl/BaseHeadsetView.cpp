#include "BaseHeadsetView.h"
#define BASE_IMPL
#include "../BaseCommon.h"

void BaseHeadsetView::SetHeadsetViewSize(uint32_t nWidth, uint32_t nHeight)
{
}

void BaseHeadsetView::GetHeadsetViewSize(uint32_t* pnWidth, uint32_t* pnHeight)
{
	*pnWidth = 0;
	*pnHeight = 0;
}

void BaseHeadsetView::SetHeadsetViewMode(OOVR_HeadsetViewMode_t eHeadsetViewMode)
{
}

OOVR_HeadsetViewMode_t BaseHeadsetView::GetHeadsetViewMode()
{
	return HeadsetViewMode_Both;
}

void BaseHeadsetView::SetHeadsetViewCropped(bool bCropped)
{
}

bool BaseHeadsetView::GetHeadsetViewCropped()
{
	return false;
}

float BaseHeadsetView::GetHeadsetViewAspectRatio()
{
	return 0.0;
}

void BaseHeadsetView::SetHeadsetViewBlendRange(float flStartPct, float flEndPct)
{
}

void BaseHeadsetView::GetHeadsetViewBlendRange(float* pStartPct, float* pEndPct)
{
	STUBBED();
}
