#include "stdafx.h"
#define CVR_IMPL
#include "CVRRenderModels005.h"
CVR_GEN_IMPL(CVRRenderModels_005);

EVRRenderModelError CVRRenderModels_005::LoadRenderModel_Async(const char * pchRenderModelName, RenderModel_t ** renderModel) {
	*renderModel = new RenderModel_t();
	// TODO use Oculus Avatars SDK

	return VRRenderModelError_None;
}

void CVRRenderModels_005::FreeRenderModel(RenderModel_t * renderModel) {
	// TODO
	delete renderModel;
}

EVRRenderModelError CVRRenderModels_005::LoadTexture_Async(TextureID_t textureId, RenderModel_TextureMap_t ** texture) {
	*texture = new RenderModel_TextureMap_t();
	// TODO use Oculus Avatars SDK

	return VRRenderModelError_None;
}

void CVRRenderModels_005::FreeTexture(RenderModel_TextureMap_t * texture) {
	// TODO
	delete texture;
}

EVRRenderModelError CVRRenderModels_005::LoadTextureD3D11_Async(TextureID_t textureId, void * pD3D11Device, void ** ppD3D11Texture2D) {
	throw "stub";
}

EVRRenderModelError CVRRenderModels_005::LoadIntoTextureD3D11_Async(TextureID_t textureId, void * pDstTexture) {
	throw "stub";
}

void CVRRenderModels_005::FreeTextureD3D11(void * pD3D11Texture2D) {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetRenderModelName(uint32_t unRenderModelIndex, VR_OUT_STRING() char * pchRenderModelName, uint32_t unRenderModelNameLen) {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetRenderModelCount() {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetComponentCount(const char * pchRenderModelName) {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetComponentName(const char * pchRenderModelName, uint32_t unComponentIndex, VR_OUT_STRING() char * pchComponentName, uint32_t unComponentNameLen) {
	throw "stub";
}

uint64_t CVRRenderModels_005::GetComponentButtonMask(const char * pchRenderModelName, const char * pchComponentName) {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetComponentRenderModelName(const char * pchRenderModelName, const char * pchComponentName, VR_OUT_STRING() char * pchComponentRenderModelName, uint32_t unComponentRenderModelNameLen) {
	throw "stub";
}

bool CVRRenderModels_005::GetComponentState(const char * pchRenderModelName, const char * pchComponentName, const vr::VRControllerState_t * pControllerState, const RenderModel_ControllerMode_State_t * pState, RenderModel_ComponentState_t * pComponentState) {
	throw "stub";
}

bool CVRRenderModels_005::RenderModelHasComponent(const char * pchRenderModelName, const char * pchComponentName) {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetRenderModelThumbnailURL(const char * pchRenderModelName, VR_OUT_STRING() char * pchThumbnailURL, uint32_t unThumbnailURLLen, EVRRenderModelError * peError) {
	throw "stub";
}

uint32_t CVRRenderModels_005::GetRenderModelOriginalPath(const char * pchRenderModelName, VR_OUT_STRING() char * pchOriginalPath, uint32_t unOriginalPathLen, EVRRenderModelError * peError) {
	throw "stub";
}

const char * CVRRenderModels_005::GetRenderModelErrorNameFromEnum(EVRRenderModelError error) {
	throw "stub";
}
