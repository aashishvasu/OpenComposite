#pragma once
#include "OpenVR/vrtypes.h"
#include "OpenVR/vrannotation.h"

// ivrrendermodels.h
namespace vr {

	static const char * const k_pch_Controller_Component_GDC2015 = "gdc2015";   // Canonical coordinate system of the gdc 2015 wired controller, provided for backwards compatibility
	static const char * const k_pch_Controller_Component_Base = "base";         // For controllers with an unambiguous 'base'.
	static const char * const k_pch_Controller_Component_Tip = "tip";           // For controllers with an unambiguous 'tip' (used for 'laser-pointing')
	static const char * const k_pch_Controller_Component_HandGrip = "handgrip"; // Neutral, ambidextrous hand-pose when holding controller. On plane between neutrally posed index finger and thumb
	static const char * const k_pch_Controller_Component_Status = "status";		// 1:1 aspect ratio status area, with canonical [0,1] uv mapping

#pragma pack( push, 8 )

																				/** Errors that can occur with the VR compositor */
	enum EVRRenderModelError {
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

	typedef uint32_t VRComponentProperties;

	enum EVRComponentProperty {
		VRComponentProperty_IsStatic = (1 << 0),
		VRComponentProperty_IsVisible = (1 << 1),
		VRComponentProperty_IsTouched = (1 << 2),
		VRComponentProperty_IsPressed = (1 << 3),
		VRComponentProperty_IsScrolled = (1 << 4),
	};

	/** Describes state information about a render-model component, including transforms and other dynamic properties */
	struct RenderModel_ComponentState_t {
		HmdMatrix34_t mTrackingToComponentRenderModel;  // Transform required when drawing the component render model
		HmdMatrix34_t mTrackingToComponentLocal;        // Transform available for attaching to a local component coordinate system (-Z out from surface )
		VRComponentProperties uProperties;
	};

	/** A single vertex in a render model */
	struct RenderModel_Vertex_t {
		HmdVector3_t vPosition;		// position in meters in device space
		HmdVector3_t vNormal;
		float rfTextureCoord[2];
	};

	/** A texture map for use on a render model */
#if defined(__linux__) || defined(__APPLE__) 
	// This structure was originally defined mis-packed on Linux, preserved for 
	// compatibility. 
#pragma pack( push, 4 )
#endif

	struct RenderModel_TextureMap_t {
		uint16_t unWidth, unHeight; // width and height of the texture map in pixels
		const uint8_t *rubTextureMapData;	// Map texture data. All textures are RGBA with 8 bits per channel per pixel. Data size is width * height * 4ub
	};
#if defined(__linux__) || defined(__APPLE__) 
#pragma pack( pop )
#endif

	/**  Session unique texture identifier. Rendermodels which share the same texture will have the same id.
	IDs <0 denote the texture is not present */

	typedef int32_t TextureID_t;

	const TextureID_t INVALID_TEXTURE_ID = -1;

#if defined(__linux__) || defined(__APPLE__) 
	// This structure was originally defined mis-packed on Linux, preserved for 
	// compatibility. 
#pragma pack( push, 4 )
#endif

	struct RenderModel_t {
		const RenderModel_Vertex_t *rVertexData;	// Vertex data for the mesh
		uint32_t unVertexCount;						// Number of vertices in the vertex data
		const uint16_t *rIndexData;					// Indices into the vertex data for each triangle
		uint32_t unTriangleCount;					// Number of triangles in the mesh. Index count is 3 * TriangleCount
		TextureID_t diffuseTextureId;				// Session unique texture identifier. Rendermodels which share the same texture will have the same id. <0 == texture not present
	};
#if defined(__linux__) || defined(__APPLE__) 
#pragma pack( pop )
#endif


	struct RenderModel_ControllerMode_State_t {
		bool bScrollWheelVisible; // is this controller currently set to be in a scroll wheel mode
	};

#pragma pack( pop )

	class IVRRenderModels {
	public:

		/** Loads and returns a render model for use in the application. pchRenderModelName should be a render model name
		* from the Prop_RenderModelName_String property or an absolute path name to a render model on disk.
		*
		* The resulting render model is valid until VR_Shutdown() is called or until FreeRenderModel() is called. When the
		* application is finished with the render model it should call FreeRenderModel() to free the memory associated
		* with the model.
		*
		* The method returns VRRenderModelError_Loading while the render model is still being loaded.
		* The method returns VRRenderModelError_None once loaded successfully, otherwise will return an error. */
		virtual EVRRenderModelError LoadRenderModel_Async(const char *pchRenderModelName, RenderModel_t **ppRenderModel) = 0;

		/** Frees a previously returned render model
		*   It is safe to call this on a null ptr. */
		virtual void FreeRenderModel(RenderModel_t *pRenderModel) = 0;

		/** Loads and returns a texture for use in the application. */
		virtual EVRRenderModelError LoadTexture_Async(TextureID_t textureId, RenderModel_TextureMap_t **ppTexture) = 0;

		/** Frees a previously returned texture
		*   It is safe to call this on a null ptr. */
		virtual void FreeTexture(RenderModel_TextureMap_t *pTexture) = 0;

		/** Creates a D3D11 texture and loads data into it. */
		virtual EVRRenderModelError LoadTextureD3D11_Async(TextureID_t textureId, void *pD3D11Device, void **ppD3D11Texture2D) = 0;

		/** Helper function to copy the bits into an existing texture. */
		virtual EVRRenderModelError LoadIntoTextureD3D11_Async(TextureID_t textureId, void *pDstTexture) = 0;

		/** Use this to free textures created with LoadTextureD3D11_Async instead of calling Release on them. */
		virtual void FreeTextureD3D11(void *pD3D11Texture2D) = 0;

		/** Use this to get the names of available render models.  Index does not correlate to a tracked device index, but
		* is only used for iterating over all available render models.  If the index is out of range, this function will return 0.
		* Otherwise, it will return the size of the buffer required for the name. */
		virtual uint32_t GetRenderModelName(uint32_t unRenderModelIndex, VR_OUT_STRING() char *pchRenderModelName, uint32_t unRenderModelNameLen) = 0;

		/** Returns the number of available render models. */
		virtual uint32_t GetRenderModelCount() = 0;


		/** Returns the number of components of the specified render model.
		*  Components are useful when client application wish to draw, label, or otherwise interact with components of tracked objects.
		*  Examples controller components:
		*   renderable things such as triggers, buttons
		*   non-renderable things which include coordinate systems such as 'tip', 'base', a neutral controller agnostic hand-pose
		*   If all controller components are enumerated and rendered, it will be equivalent to drawing the traditional render model
		*   Returns 0 if components not supported, >0 otherwise */
		virtual uint32_t GetComponentCount(const char *pchRenderModelName) = 0;

		/** Use this to get the names of available components.  Index does not correlate to a tracked device index, but
		* is only used for iterating over all available components.  If the index is out of range, this function will return 0.
		* Otherwise, it will return the size of the buffer required for the name. */
		virtual uint32_t GetComponentName(const char *pchRenderModelName, uint32_t unComponentIndex, VR_OUT_STRING() char *pchComponentName, uint32_t unComponentNameLen) = 0;

		/** Get the button mask for all buttons associated with this component
		*   If no buttons (or axes) are associated with this component, return 0
		*   Note: multiple components may be associated with the same button. Ex: two grip buttons on a single controller.
		*   Note: A single component may be associated with multiple buttons. Ex: A trackpad which also provides "D-pad" functionality */
		virtual uint64_t GetComponentButtonMask(const char *pchRenderModelName, const char *pchComponentName) = 0;

		/** Use this to get the render model name for the specified rendermode/component combination, to be passed to LoadRenderModel.
		* If the component name is out of range, this function will return 0.
		* Otherwise, it will return the size of the buffer required for the name. */
		virtual uint32_t GetComponentRenderModelName(const char *pchRenderModelName, const char *pchComponentName, VR_OUT_STRING() char *pchComponentRenderModelName, uint32_t unComponentRenderModelNameLen) = 0;

		/** Use this to query information about the component, as a function of the controller state.
		*
		* For dynamic controller components (ex: trigger) values will reflect component motions
		* For static components this will return a consistent value independent of the VRControllerState_t
		*
		* If the pchRenderModelName or pchComponentName is invalid, this will return false (and transforms will be set to identity).
		* Otherwise, return true
		* Note: For dynamic objects, visibility may be dynamic. (I.e., true/false will be returned based on controller state and controller mode state ) */
		virtual bool GetComponentState(const char *pchRenderModelName, const char *pchComponentName, const vr::VRControllerState_t *pControllerState, const RenderModel_ControllerMode_State_t *pState, RenderModel_ComponentState_t *pComponentState) = 0;

		/** Returns true if the render model has a component with the specified name */
		virtual bool RenderModelHasComponent(const char *pchRenderModelName, const char *pchComponentName) = 0;

		/** Returns the URL of the thumbnail image for this rendermodel */
		virtual uint32_t GetRenderModelThumbnailURL(const char *pchRenderModelName, VR_OUT_STRING() char *pchThumbnailURL, uint32_t unThumbnailURLLen, vr::EVRRenderModelError *peError) = 0;

		/** Provides a render model path that will load the unskinned model if the model name provided has been replace by the user. If the model
		* hasn't been replaced the path value will still be a valid path to load the model. Pass this to LoadRenderModel_Async, etc. to load the
		* model. */
		virtual uint32_t GetRenderModelOriginalPath(const char *pchRenderModelName, VR_OUT_STRING() char *pchOriginalPath, uint32_t unOriginalPathLen, vr::EVRRenderModelError *peError) = 0;

		/** Returns a string for a render model error */
		virtual const char *GetRenderModelErrorNameFromEnum(vr::EVRRenderModelError error) = 0;
	};

	static const char * const IVRRenderModels_Version = "IVRRenderModels_005";

}

