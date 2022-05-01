#include "stdafx.h"
#define GENFILE
#include "BaseCommon.h"

GEN_INTERFACE("RenderModels", "002")
GEN_INTERFACE("RenderModels", "004")
GEN_INTERFACE("RenderModels", "005")
GEN_INTERFACE("RenderModels", "006")

#include "GVRRenderModels.gen.h"

bool CVRRenderModels_002::LoadRenderModel(const char* pchRenderModelName, vr::IVRRenderModels_002::RenderModel_t** ppRenderModel) 
{
	return base->LoadRenderModel_Async(pchRenderModelName, (OOVR_RenderModel_t**)ppRenderModel); 
}

bool CVRRenderModels_002::LoadTexture(vr::IVRRenderModels_002::TextureID_t textureId, vr::IVRRenderModels_002::RenderModel_TextureMap_t** ppTexture)
{ 
	return base->LoadTexture_Async((OOVR_TextureID_t)textureId, (OOVR_RenderModel_TextureMap_t**)ppTexture); 
}

bool CVRRenderModels_002::GetComponentState(const char* pchRenderModelName, const char* pchComponentName, const vr::VRControllerState_t* pControllerState, vr::IVRRenderModels_002::RenderModel_ComponentState_t* pComponentState) 
{
	return base->GetComponentState(pchRenderModelName, pchComponentName, pControllerState, nullptr, (OOVR_RenderModel_ComponentState_t*)pComponentState);
}
