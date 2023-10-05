#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("Overlay", "007")
GEN_INTERFACE("Overlay", "010")
GEN_INTERFACE("Overlay", "011")
// TODO 012
GEN_INTERFACE("Overlay", "013")
GEN_INTERFACE("Overlay", "014")
// TODO 015
GEN_INTERFACE("Overlay", "016")
GEN_INTERFACE("Overlay", "017")
GEN_INTERFACE("Overlay", "018")
GEN_INTERFACE("Overlay", "019")
GEN_INTERFACE("Overlay", "020")
GEN_INTERFACE("Overlay", "021")
GEN_INTERFACE("Overlay", "022")
// Version 023 never appears in a public release
GEN_INTERFACE("Overlay", "024")
GEN_INTERFACE("Overlay", "025")
GEN_INTERFACE("Overlay", "026")

#include "generated/GVROverlay.gen.h"

bool CVROverlay_007::PollNextOverlayEvent(vr::VROverlayHandle_t ulOverlayHandle, vr::VREvent_t* pEvent)
{
	return base->PollNextOverlayEvent(ulOverlayHandle, pEvent, sizeof(vr::VREvent_t));
}

vr::EVROverlayError CVROverlay_011::GetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle, void** pNativeTextureHandle,
    void* pNativeTextureRef, uint32_t* pWidth, uint32_t* pHeight, uint32_t* pNativeFormat, EGraphicsAPIConvention* pAPI,
    vr::EColorSpace* pColorSpace)
{

	// It should be fairly simple, but it's unlikely to be used so I can't be bothered implementing it now
	STUBBED();
}

vr::EVROverlayError CVROverlay_013::GetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle, void** pNativeTextureHandle,
    void* pNativeTextureRef, uint32_t* pWidth, uint32_t* pHeight, uint32_t* pNativeFormat, EGraphicsAPIConvention* pAPI,
    vr::EColorSpace* pColorSpace)
{

	// It should be fairly simple, but it's unlikely to be used so I can't be bothered implementing it now
	STUBBED();
}
