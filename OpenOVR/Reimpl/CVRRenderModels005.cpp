#include "stdafx.h"
#define CVR_IMPL
#include "CVRRenderModels005.h"

CVR_GEN_IMPL(CVRRenderModels_005);

EVRRenderModelError CVRRenderModels_005::LoadRenderModel_Async(const char * pchRenderModelName, RenderModel_t ** ppRenderModel) {
	return (EVRRenderModelError) base.LoadRenderModel_Async(pchRenderModelName, (OOVR_RenderModel_t**) ppRenderModel);
}
void CVRRenderModels_005::FreeRenderModel(RenderModel_t * pRenderModel) {
	return base.FreeRenderModel((OOVR_RenderModel_t*) pRenderModel);
}
EVRRenderModelError CVRRenderModels_005::LoadTexture_Async(TextureID_t textureId, RenderModel_TextureMap_t ** ppTexture) {
	return (EVRRenderModelError)base.LoadTexture_Async(textureId, (OOVR_RenderModel_TextureMap_t**) ppTexture);
}
void CVRRenderModels_005::FreeTexture(RenderModel_TextureMap_t * pTexture) {
	return base.FreeTexture((OOVR_RenderModel_TextureMap_t*) pTexture);
}
EVRRenderModelError CVRRenderModels_005::LoadTextureD3D11_Async(TextureID_t textureId, void * pD3D11Device, void ** ppD3D11Texture2D) {
	return (EVRRenderModelError) base.LoadTextureD3D11_Async(textureId, pD3D11Device, ppD3D11Texture2D);
}
EVRRenderModelError CVRRenderModels_005::LoadIntoTextureD3D11_Async(TextureID_t textureId, void * pDstTexture) {
	return (EVRRenderModelError) base.LoadIntoTextureD3D11_Async(textureId, pDstTexture);
}
void CVRRenderModels_005::FreeTextureD3D11(void * pD3D11Texture2D) {
	return base.FreeTextureD3D11(pD3D11Texture2D);
}
uint32_t CVRRenderModels_005::GetRenderModelName(uint32_t unRenderModelIndex, VR_OUT_STRING() char * pchRenderModelName, uint32_t unRenderModelNameLen) {
	return base.GetRenderModelName(unRenderModelIndex, pchRenderModelName, unRenderModelNameLen);
}
uint32_t CVRRenderModels_005::GetRenderModelCount() {
	return base.GetRenderModelCount();
}
uint32_t CVRRenderModels_005::GetComponentCount(const char * pchRenderModelName) {
	return base.GetComponentCount(pchRenderModelName);
}
uint32_t CVRRenderModels_005::GetComponentName(const char * pchRenderModelName, uint32_t unComponentIndex, VR_OUT_STRING() char * pchComponentName, uint32_t unComponentNameLen) {
	return base.GetComponentName(pchRenderModelName, unComponentIndex, pchComponentName, unComponentNameLen);
}
uint64_t CVRRenderModels_005::GetComponentButtonMask(const char * pchRenderModelName, const char * pchComponentName) {
	return base.GetComponentButtonMask(pchRenderModelName, pchComponentName);
}
uint32_t CVRRenderModels_005::GetComponentRenderModelName(const char * pchRenderModelName, const char * pchComponentName, VR_OUT_STRING() char * pchComponentRenderModelName, uint32_t unComponentRenderModelNameLen) {
	return base.GetComponentRenderModelName(pchRenderModelName, pchComponentName, pchComponentRenderModelName, unComponentRenderModelNameLen);
}
bool CVRRenderModels_005::GetComponentState(const char * pchRenderModelName, const char * pchComponentName, const VRControllerState_t * pControllerState, const RenderModel_ControllerMode_State_t * pState, RenderModel_ComponentState_t * pComponentState) {
	return base.GetComponentState(pchRenderModelName, pchComponentName, pControllerState,
		(const OOVR_RenderModel_ControllerMode_State_t*) pState,
		(OOVR_RenderModel_ComponentState_t*) pComponentState);
}
bool CVRRenderModels_005::RenderModelHasComponent(const char * pchRenderModelName, const char * pchComponentName) {
	return base.RenderModelHasComponent(pchRenderModelName, pchComponentName);
}
uint32_t CVRRenderModels_005::GetRenderModelThumbnailURL(const char * pchRenderModelName, VR_OUT_STRING() char * pchThumbnailURL, uint32_t unThumbnailURLLen, EVRRenderModelError * peError) {
	return base.GetRenderModelThumbnailURL(pchRenderModelName, pchThumbnailURL, unThumbnailURLLen, (OOVR_EVRRenderModelError*) peError);
}
uint32_t CVRRenderModels_005::GetRenderModelOriginalPath(const char * pchRenderModelName, VR_OUT_STRING() char * pchOriginalPath, uint32_t unOriginalPathLen, EVRRenderModelError * peError) {
	return base.GetRenderModelOriginalPath(pchRenderModelName, pchOriginalPath, unOriginalPathLen, (OOVR_EVRRenderModelError*) peError);
}
const char * CVRRenderModels_005::GetRenderModelErrorNameFromEnum(EVRRenderModelError error) {
	return base.GetRenderModelErrorNameFromEnum((OOVR_EVRRenderModelError) error);
}
