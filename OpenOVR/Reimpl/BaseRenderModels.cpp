#include "stdafx.h"
#define BASE_IMPL
#include "BaseRenderModels.h"

#pragma region structs

enum OOVR_EVRRenderModelError {
	VRRenderModelError_None = 0,
	VRRenderModelError_Loading = 100,
	VRRenderModelError_NotSupported = 200,
	VRRenderModelError_InvalidArg = 300,
	VRRenderModelError_InvalidModel = 301,
	VRRenderModelError_NoShapes = 302,
	VRRenderModelError_MultipleShapes = 303,
	VRRenderModelError_TooManyVertices = 304,
	VRRenderModelError_MultipleTextures = 305,
	VRRenderModelError_BufferTooSmall = 306,
	VRRenderModelError_NotEnoughNormals = 307,
	VRRenderModelError_NotEnoughTexCoords = 308,

	VRRenderModelError_InvalidTexture = 400,
};

struct OOVR_RenderModel_Vertex_t {
	vr::HmdVector3_t vPosition;		// position in meters in device space
	vr::HmdVector3_t vNormal;
	float rfTextureCoord[2];
};

#if defined(__linux__) || defined(__APPLE__) 
// This structure was originally defined mis-packed on Linux, preserved for 
// compatibility. 
#pragma pack( push, 4 )
#endif

struct OOVR_RenderModel_t {
	const OOVR_RenderModel_Vertex_t *rVertexData;	// Vertex data for the mesh
	uint32_t unVertexCount;						// Number of vertices in the vertex data
	const uint16_t *rIndexData;					// Indices into the vertex data for each triangle
	uint32_t unTriangleCount;					// Number of triangles in the mesh. Index count is 3 * TriangleCount
	OOVR_TextureID_t diffuseTextureId;			// Session unique texture identifier. Rendermodels which share the same texture will have the same id. <0 == texture not present
};

struct OOVR_RenderModel_TextureMap_t {
	uint16_t unWidth, unHeight; // width and height of the texture map in pixels
	const uint8_t *rubTextureMapData;	// Map texture data. All textures are RGBA with 8 bits per channel per pixel. Data size is width * height * 4ub
};

#if defined(__linux__) || defined(__APPLE__) 
#pragma pack( pop )
#endif

#pragma endregion

typedef OOVR_RenderModel_t RenderModel_t;
typedef OOVR_EVRRenderModelError EVRRenderModelError;
typedef OOVR_RenderModel_TextureMap_t RenderModel_TextureMap_t;
typedef OOVR_TextureID_t TextureID_t;

EVRRenderModelError BaseRenderModels::LoadRenderModel_Async(const char * pchRenderModelName, RenderModel_t ** renderModel) {
	*renderModel = new RenderModel_t();
	// TODO use Oculus Avatars SDK

	return VRRenderModelError_None;
}

void BaseRenderModels::FreeRenderModel(RenderModel_t * renderModel) {
	// TODO
	delete renderModel;
}

EVRRenderModelError BaseRenderModels::LoadTexture_Async(TextureID_t textureId, RenderModel_TextureMap_t ** texture) {
	*texture = new RenderModel_TextureMap_t();
	// TODO use Oculus Avatars SDK

	return VRRenderModelError_None;
}

void BaseRenderModels::FreeTexture(RenderModel_TextureMap_t * texture) {
	// TODO
	delete texture;
}

EVRRenderModelError BaseRenderModels::LoadTextureD3D11_Async(TextureID_t textureId, void * pD3D11Device, void ** ppD3D11Texture2D) {
	STUBBED();
}

EVRRenderModelError BaseRenderModels::LoadIntoTextureD3D11_Async(TextureID_t textureId, void * pDstTexture) {
	STUBBED();
}

void BaseRenderModels::FreeTextureD3D11(void * pD3D11Texture2D) {
	STUBBED();
}

uint32_t BaseRenderModels::GetRenderModelName(uint32_t unRenderModelIndex, VR_OUT_STRING() char * pchRenderModelName, uint32_t unRenderModelNameLen) {
	STUBBED();
}

uint32_t BaseRenderModels::GetRenderModelCount() {
	STUBBED();
}

uint32_t BaseRenderModels::GetComponentCount(const char * pchRenderModelName) {
	STUBBED();
}

uint32_t BaseRenderModels::GetComponentName(const char * pchRenderModelName, uint32_t unComponentIndex, VR_OUT_STRING() char * pchComponentName, uint32_t unComponentNameLen) {
	STUBBED();
}

uint64_t BaseRenderModels::GetComponentButtonMask(const char * pchRenderModelName, const char * pchComponentName) {
	STUBBED();
}

uint32_t BaseRenderModels::GetComponentRenderModelName(const char * pchRenderModelName, const char * pchComponentName, VR_OUT_STRING() char * pchComponentRenderModelName, uint32_t unComponentRenderModelNameLen) {
	STUBBED();
}

bool BaseRenderModels::GetComponentState(const char * pchRenderModelName, const char * pchComponentName, const vr::VRControllerState_t * pControllerState, const OOVR_RenderModel_ControllerMode_State_t * pState, OOVR_RenderModel_ComponentState_t * pComponentState) {
	STUBBED();
}

bool BaseRenderModels::RenderModelHasComponent(const char * pchRenderModelName, const char * pchComponentName) {
	STUBBED();
}

uint32_t BaseRenderModels::GetRenderModelThumbnailURL(const char * pchRenderModelName, VR_OUT_STRING() char * pchThumbnailURL, uint32_t unThumbnailURLLen, EVRRenderModelError * peError) {
	STUBBED();
}

uint32_t BaseRenderModels::GetRenderModelOriginalPath(const char * pchRenderModelName, VR_OUT_STRING() char * pchOriginalPath, uint32_t unOriginalPathLen, EVRRenderModelError * peError) {
	STUBBED();
}

const char * BaseRenderModels::GetRenderModelErrorNameFromEnum(EVRRenderModelError error) {
	STUBBED();
}
