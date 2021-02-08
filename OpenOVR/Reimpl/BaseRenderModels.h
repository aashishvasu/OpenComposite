#pragma once
#include "BaseCommon.h"

enum OOVR_EVRRenderModelError : int;
struct OOVR_RenderModel_t;
typedef int32_t OOVR_TextureID_t;
struct OOVR_RenderModel_TextureMap_t;
struct OOVR_RenderModel_ControllerMode_State_t;
typedef vr::RenderModel_ComponentState_t OOVR_RenderModel_ComponentState_t;

class BaseRenderModels {
private:

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
	virtual OOVR_EVRRenderModelError LoadRenderModel_Async(const char *pchRenderModelName, OOVR_RenderModel_t **ppRenderModel);

	/** Frees a previously returned render model
	*   It is safe to call this on a null ptr. */
	virtual void FreeRenderModel(OOVR_RenderModel_t *pRenderModel);

	/** Loads and returns a texture for use in the application. */
	virtual OOVR_EVRRenderModelError LoadTexture_Async(OOVR_TextureID_t textureId, OOVR_RenderModel_TextureMap_t **ppTexture);

	/** Frees a previously returned texture
	*   It is safe to call this on a null ptr. */
	virtual void FreeTexture(OOVR_RenderModel_TextureMap_t *pTexture);

	/** Creates a D3D11 texture and loads data into it. */
	virtual OOVR_EVRRenderModelError LoadTextureD3D11_Async(OOVR_TextureID_t textureId, void *pD3D11Device, void **ppD3D11Texture2D);

	/** Helper function to copy the bits into an existing texture. */
	virtual OOVR_EVRRenderModelError LoadIntoTextureD3D11_Async(OOVR_TextureID_t textureId, void *pDstTexture);

	/** Use this to free textures created with LoadTextureD3D11_Async instead of calling Release on them. */
	virtual void FreeTextureD3D11(void *pD3D11Texture2D);

	/** Use this to get the names of available render models.  Index does not correlate to a tracked device index, but
	* is only used for iterating over all available render models.  If the index is out of range, this function will return 0.
	* Otherwise, it will return the size of the buffer required for the name. */
	virtual uint32_t GetRenderModelName(uint32_t unRenderModelIndex, VR_OUT_STRING() char *pchRenderModelName, uint32_t unRenderModelNameLen);

	/** Returns the number of available render models. */
	virtual uint32_t GetRenderModelCount();


	/** Returns the number of components of the specified render model.
	*  Components are useful when client application wish to draw, label, or otherwise interact with components of tracked objects.
	*  Examples controller components:
	*   renderable things such as triggers, buttons
	*   non-renderable things which include coordinate systems such as 'tip', 'base', a neutral controller agnostic hand-pose
	*   If all controller components are enumerated and rendered, it will be equivalent to drawing the traditional render model
	*   Returns 0 if components not supported, >0 otherwise */
	virtual uint32_t GetComponentCount(const char *pchRenderModelName);

	/** Use this to get the names of available components.  Index does not correlate to a tracked device index, but
	* is only used for iterating over all available components.  If the index is out of range, this function will return 0.
	* Otherwise, it will return the size of the buffer required for the name. */
	virtual uint32_t GetComponentName(const char *pchRenderModelName, uint32_t unComponentIndex, VR_OUT_STRING() char *pchComponentName, uint32_t unComponentNameLen);

	/** Get the button mask for all buttons associated with this component
	*   If no buttons (or axes) are associated with this component, return 0
	*   Note: multiple components may be associated with the same button. Ex: two grip buttons on a single controller.
	*   Note: A single component may be associated with multiple buttons. Ex: A trackpad which also provides "D-pad" functionality */
	virtual uint64_t GetComponentButtonMask(const char *pchRenderModelName, const char *pchComponentName);

	/** Use this to get the render model name for the specified rendermode/component combination, to be passed to LoadRenderModel.
	* If the component name is out of range, this function will return 0.
	* Otherwise, it will return the size of the buffer required for the name. */
	virtual uint32_t GetComponentRenderModelName(const char *pchRenderModelName, const char *pchComponentName, VR_OUT_STRING() char *pchComponentRenderModelName, uint32_t unComponentRenderModelNameLen);

	/** Use this to query information about the component, as a function of the controller state.
	*
	* For dynamic controller components (ex: trigger) values will reflect component motions
	* For static components this will return a consistent value independent of the VRControllerState_t
	*
	* If the pchRenderModelName or pchComponentName is invalid, this will return false (and transforms will be set to identity).
	* Otherwise, return true
	* Note: For dynamic objects, visibility may be dynamic. (I.e., true/false will be returned based on controller state and controller mode state ) */
	virtual bool GetComponentStateForDevicePath(const char *pchRenderModelName, const char *pchComponentName, vr::VRInputValueHandle_t devicePath, const OOVR_RenderModel_ControllerMode_State_t *pState, OOVR_RenderModel_ComponentState_t *pComponentState);

	/** This version of GetComponentState takes a controller state block instead of an action origin. This function is deprecated. You should use the new input system and GetComponentStateForDevicePath instead. */
	virtual bool GetComponentState(const char *pchRenderModelName, const char *pchComponentName, const vr::VRControllerState_t *pControllerState, const OOVR_RenderModel_ControllerMode_State_t *pState, OOVR_RenderModel_ComponentState_t *pComponentState);

	/** Returns true if the render model has a component with the specified name */
	virtual bool RenderModelHasComponent(const char *pchRenderModelName, const char *pchComponentName);

	/** Returns the URL of the thumbnail image for this rendermodel */
	virtual uint32_t GetRenderModelThumbnailURL(const char *pchRenderModelName, VR_OUT_STRING() char *pchThumbnailURL, uint32_t unThumbnailURLLen, OOVR_EVRRenderModelError *peError);

	/** Provides a render model path that will load the unskinned model if the model name provided has been replace by the user. If the model
	* hasn't been replaced the path value will still be a valid path to load the model. Pass this to LoadRenderModel_Async, etc. to load the
	* model. */
	virtual uint32_t GetRenderModelOriginalPath(const char *pchRenderModelName, VR_OUT_STRING() char *pchOriginalPath, uint32_t unOriginalPathLen, OOVR_EVRRenderModelError *peError);

	/** Returns a string for a render model error */
	virtual const char *GetRenderModelErrorNameFromEnum(OOVR_EVRRenderModelError error);
};
