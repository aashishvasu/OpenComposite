#pragma once
#include "OpenVR/interfaces/vrtypes.h"

using namespace vr; // TODO eliminate this

class BaseOverlay {
private:
	// These enums are ints
	typedef int VROverlayFlags;
	typedef int VROverlayTransformType;
	typedef int VROverlayInputMethod;
	typedef int EOverlayDirection;
	typedef int EGamepadTextInputMode;
	typedef int EGamepadTextInputLineMode;
	typedef int VRMessageOverlayResponse;

public:

	// ---------------------------------------------
	// Overlay management methods
	// ---------------------------------------------

	/** Finds an existing overlay with the specified key. */
	virtual EVROverlayError FindOverlay(const char *pchOverlayKey, VROverlayHandle_t * pOverlayHandle);

	/** Creates a new named overlay. All overlays start hidden and with default settings. */
	virtual EVROverlayError CreateOverlay(const char *pchOverlayKey, const char *pchOverlayName, VROverlayHandle_t * pOverlayHandle);

	/** Destroys the specified overlay. When an application calls VR_Shutdown all overlays created by that app are
	* automatically destroyed. */
	virtual EVROverlayError DestroyOverlay(VROverlayHandle_t ulOverlayHandle);

	/** Specify which overlay to use the high quality render path.  This overlay will be composited in during the distortion pass which
	* results in it drawing on top of everything else, but also at a higher quality as it samples the source texture directly rather than
	* rasterizing into each eye's render texture first.  Because if this, only one of these is supported at any given time.  It is most useful
	* for overlays that are expected to take up most of the user's view (e.g. streaming video).
	* This mode does not support mouse input to your overlay. */
	virtual EVROverlayError SetHighQualityOverlay(VROverlayHandle_t ulOverlayHandle);

	/** Returns the overlay handle of the current overlay being rendered using the single high quality overlay render path.
	* Otherwise it will return k_ulOverlayHandleInvalid. */
	virtual VROverlayHandle_t GetHighQualityOverlay();

	/** Fills the provided buffer with the string key of the overlay. Returns the size of buffer required to store the key, including
	* the terminating null character. k_unVROverlayMaxKeyLength will be enough bytes to fit the string. */
	virtual uint32_t GetOverlayKey(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, EVROverlayError *pError = 0L);

	/** Fills the provided buffer with the friendly name of the overlay. Returns the size of buffer required to store the key, including
	* the terminating null character. k_unVROverlayMaxNameLength will be enough bytes to fit the string. */
	virtual uint32_t GetOverlayName(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, EVROverlayError *pError = 0L);

	/** set the name to use for this overlay */
	virtual EVROverlayError SetOverlayName(VROverlayHandle_t ulOverlayHandle, const char *pchName);

	/** Gets the raw image data from an overlay. Overlay image data is always returned as RGBA data, 4 bytes per pixel. If the buffer is not large enough, width and height
	* will be set and VROverlayError_ArrayTooSmall is returned. */
	virtual EVROverlayError GetOverlayImageData(VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unBufferSize, uint32_t *punWidth, uint32_t *punHeight);

	/** returns a string that corresponds with the specified overlay error. The string will be the name
	* of the error enum value for all valid error codes */
	virtual const char *GetOverlayErrorNameFromEnum(EVROverlayError error);

	// ---------------------------------------------
	// Overlay rendering methods
	// ---------------------------------------------

	/** Sets the pid that is allowed to render to this overlay (the creator pid is always allow to render),
	*	by default this is the pid of the process that made the overlay */
	virtual EVROverlayError SetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle, uint32_t unPID);

	/** Gets the pid that is allowed to render to this overlay */
	virtual uint32_t GetOverlayRenderingPid(VROverlayHandle_t ulOverlayHandle);

	/** Specify flag setting for a given overlay */
	virtual EVROverlayError SetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled);

	/** Sets flag setting for a given overlay */
	virtual EVROverlayError GetOverlayFlag(VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool *pbEnabled);

	/** Sets the color tint of the overlay quad. Use 0.0 to 1.0 per channel. */
	virtual EVROverlayError SetOverlayColor(VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue);

	/** Gets the color tint of the overlay quad. */
	virtual EVROverlayError GetOverlayColor(VROverlayHandle_t ulOverlayHandle, float *pfRed, float *pfGreen, float *pfBlue);

	/** Sets the alpha of the overlay quad. Use 1.0 for 100 percent opacity to 0.0 for 0 percent opacity. */
	virtual EVROverlayError SetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float fAlpha);

	/** Gets the alpha of the overlay quad. By default overlays are rendering at 100 percent alpha (1.0). */
	virtual EVROverlayError GetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float *pfAlpha);

	/** Sets the aspect ratio of the texels in the overlay. 1.0 means the texels are square. 2.0 means the texels
	* are twice as wide as they are tall. Defaults to 1.0. */
	virtual EVROverlayError SetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float fTexelAspect);

	/** Gets the aspect ratio of the texels in the overlay. Defaults to 1.0 */
	virtual EVROverlayError GetOverlayTexelAspect(VROverlayHandle_t ulOverlayHandle, float *pfTexelAspect);

	/** Sets the rendering sort order for the overlay. Overlays are rendered this order:
	*      Overlays owned by the scene application
	*      Overlays owned by some other application
	*
	*	Within a category overlays are rendered lowest sort order to highest sort order. Overlays with the same
	*	sort order are rendered back to front base on distance from the HMD.
	*
	*	Sort order defaults to 0. */
	virtual EVROverlayError SetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder);

	/** Gets the sort order of the overlay. See SetOverlaySortOrder for how this works. */
	virtual EVROverlayError GetOverlaySortOrder(VROverlayHandle_t ulOverlayHandle, uint32_t *punSortOrder);

	/** Sets the width of the overlay quad in meters. By default overlays are rendered on a quad that is 1 meter across */
	virtual EVROverlayError SetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float fWidthInMeters);

	/** Returns the width of the overlay quad in meters. By default overlays are rendered on a quad that is 1 meter across */
	virtual EVROverlayError GetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float *pfWidthInMeters);

	/** For high-quality curved overlays only, sets the distance range in meters from the overlay used to automatically curve
	* the surface around the viewer.  Min is distance is when the surface will be most curved.  Max is when least curved. */
	virtual EVROverlayError SetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters);

	/** For high-quality curved overlays only, gets the distance range in meters from the overlay used to automatically curve
	* the surface around the viewer.  Min is distance is when the surface will be most curved.  Max is when least curved. */
	virtual EVROverlayError GetOverlayAutoCurveDistanceRangeInMeters(VROverlayHandle_t ulOverlayHandle, float *pfMinDistanceInMeters, float *pfMaxDistanceInMeters);

	/** Sets the colorspace the overlay texture's data is in.  Defaults to 'auto'.
	* If the texture needs to be resolved, you should call SetOverlayTexture with the appropriate colorspace instead. */
	virtual EVROverlayError SetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace);

	/** Gets the overlay's current colorspace setting. */
	virtual EVROverlayError GetOverlayTextureColorSpace(VROverlayHandle_t ulOverlayHandle, EColorSpace *peTextureColorSpace);

	/** Sets the part of the texture to use for the overlay. UV Min is the upper left corner and UV Max is the lower right corner. */
	virtual EVROverlayError SetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, const VRTextureBounds_t *pOverlayTextureBounds);

	/** Gets the part of the texture to use for the overlay. UV Min is the upper left corner and UV Max is the lower right corner. */
	virtual EVROverlayError GetOverlayTextureBounds(VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t *pOverlayTextureBounds);

	/** Gets render model to draw behind this overlay */
	virtual uint32_t GetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, HmdColor_t *pColor, EVROverlayError *pError);

	/** Sets render model to draw behind this overlay and the vertex color to use, pass null for pColor to match the overlays vertex color.
	The model is scaled by the same amount as the overlay, with a default of 1m. */
	virtual EVROverlayError SetOverlayRenderModel(VROverlayHandle_t ulOverlayHandle, const char *pchRenderModel, const HmdColor_t *pColor);

	/** Returns the transform type of this overlay. */
	virtual EVROverlayError GetOverlayTransformType(VROverlayHandle_t ulOverlayHandle, VROverlayTransformType *peTransformType);

	/** Sets the transform to absolute tracking origin. */
	virtual EVROverlayError SetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToOverlayTransform);

	/** Gets the transform if it is absolute. Returns an error if the transform is some other type. */
	virtual EVROverlayError GetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin *peTrackingOrigin, HmdMatrix34_t *pmatTrackingOriginToOverlayTransform);

	/** Sets the transform to relative to the transform of the specified tracked device. */
	virtual EVROverlayError SetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, const HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform);

	/** Gets the transform if it is relative to a tracked device. Returns an error if the transform is some other type. */
	virtual EVROverlayError GetOverlayTransformTrackedDeviceRelative(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punTrackedDevice, HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform);

	/** Sets the transform to draw the overlay on a rendermodel component mesh instead of a quad. This will only draw when the system is
	* drawing the device. Overlays with this transform type cannot receive mouse events. */
	virtual EVROverlayError SetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, const char *pchComponentName);

	/** Gets the transform information when the overlay is rendering on a component. */
	virtual EVROverlayError GetOverlayTransformTrackedDeviceComponent(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punDeviceIndex, char *pchComponentName, uint32_t unComponentNameSize);

	/** Gets the transform if it is relative to another overlay. Returns an error if the transform is some other type. */
	virtual EVROverlayError GetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t *ulOverlayHandleParent, HmdMatrix34_t *pmatParentOverlayToOverlayTransform);

	/** Sets the transform to relative to the transform of the specified overlay. This overlays visibility will also track the parents visibility */
	virtual EVROverlayError SetOverlayTransformOverlayRelative(VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, const HmdMatrix34_t *pmatParentOverlayToOverlayTransform);

	/** Shows the VR overlay.  For dashboard overlays, only the Dashboard Manager is allowed to call this. */
	virtual EVROverlayError ShowOverlay(VROverlayHandle_t ulOverlayHandle);

	/** Hides the VR overlay.  For dashboard overlays, only the Dashboard Manager is allowed to call this. */
	virtual EVROverlayError HideOverlay(VROverlayHandle_t ulOverlayHandle);

	/** Returns true if the overlay is visible. */
	virtual bool IsOverlayVisible(VROverlayHandle_t ulOverlayHandle);

	/** Get the transform in 3d space associated with a specific 2d point in the overlay's coordinate space (where 0,0 is the lower left). -Z points out of the overlay */
	virtual EVROverlayError GetTransformForOverlayCoordinates(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay, HmdMatrix34_t *pmatTransform);

	// ---------------------------------------------
	// Overlay input methods
	// ---------------------------------------------

	/** Returns true and fills the event with the next event on the overlay's event queue, if there is one.
	* If there are no events this method returns false. uncbVREvent should be the size in bytes of the VREvent_t struct */
	virtual bool PollNextOverlayEvent(VROverlayHandle_t ulOverlayHandle, VREvent_t *pEvent, uint32_t uncbVREvent);

	/** Returns the current input settings for the specified overlay. */
	virtual EVROverlayError GetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod *peInputMethod);

	/** Sets the input settings for the specified overlay. */
	virtual EVROverlayError SetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod);

	/** Gets the mouse scaling factor that is used for mouse events. The actual texture may be a different size, but this is
	* typically the size of the underlying UI in pixels. */
	virtual EVROverlayError GetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, HmdVector2_t *pvecMouseScale);

	/** Sets the mouse scaling factor that is used for mouse events. The actual texture may be a different size, but this is
	* typically the size of the underlying UI in pixels (not in world space). */
	virtual EVROverlayError SetOverlayMouseScale(VROverlayHandle_t ulOverlayHandle, const HmdVector2_t *pvecMouseScale);

	/** Computes the overlay-space pixel coordinates of where the ray intersects the overlay with the
	* specified settings. Returns false if there is no intersection. */
	// TODO structs
	//virtual bool ComputeOverlayIntersection(VROverlayHandle_t ulOverlayHandle, const VROverlayIntersectionParams_t *pParams, VROverlayIntersectionResults_t *pResults);

	/** Processes mouse input from the specified controller as though it were a mouse pointed at a compositor overlay with the
	* specified settings. The controller is treated like a laser pointer on the -z axis. The point where the laser pointer would
	* intersect with the overlay is the mouse position, the trigger is left mouse, and the track pad is right mouse.
	*
	* Return true if the controller is pointed at the overlay and an event was generated. */
	virtual bool HandleControllerOverlayInteractionAsMouse(VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unControllerDeviceIndex);

	/** Returns true if the specified overlay is the hover target. An overlay is the hover target when it is the last overlay "moused over"
	* by the virtual mouse pointer */
	virtual bool IsHoverTargetOverlay(VROverlayHandle_t ulOverlayHandle);

	/** Returns the current Gamepad focus overlay */
	virtual VROverlayHandle_t GetGamepadFocusOverlay();

	/** Sets the current Gamepad focus overlay */
	virtual EVROverlayError SetGamepadFocusOverlay(VROverlayHandle_t ulNewFocusOverlay);

	/** Sets an overlay's neighbor. This will also set the neighbor of the "to" overlay
	* to point back to the "from" overlay. If an overlay's neighbor is set to invalid both
	* ends will be cleared */
	virtual EVROverlayError SetOverlayNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo);

	/** Changes the Gamepad focus from one overlay to one of its neighbors. Returns VROverlayError_NoNeighbor if there is no
	* neighbor in that direction */
	virtual EVROverlayError MoveGamepadFocusToNeighbor(EOverlayDirection eDirection, VROverlayHandle_t ulFrom);

	/** Sets the analog input to Dual Analog coordinate scale for the specified overlay. */
	virtual EVROverlayError SetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const HmdVector2_t & vCenter, float fRadius);

	/** Gets the analog input to Dual Analog coordinate scale for the specified overlay. */
	virtual EVROverlayError GetOverlayDualAnalogTransform(VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, HmdVector2_t *pvCenter, float *pfRadius);

	// ---------------------------------------------
	// Overlay texture methods
	// ---------------------------------------------

	/** Texture to draw for the overlay. This function can only be called by the overlay's creator or renderer process (see SetOverlayRenderingPid) .
	*
	* OpenGL dirty state:
	*	glBindTexture
	*/
	virtual EVROverlayError SetOverlayTexture(VROverlayHandle_t ulOverlayHandle, const Texture_t *pTexture);

	/** Use this to tell the overlay system to release the texture set for this overlay. */
	virtual EVROverlayError ClearOverlayTexture(VROverlayHandle_t ulOverlayHandle);

	/** Separate interface for providing the data as a stream of bytes, but there is an upper bound on data
	* that can be sent. This function can only be called by the overlay's renderer process. */
	virtual EVROverlayError SetOverlayRaw(VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth);

	/** Separate interface for providing the image through a filename: can be png or jpg, and should not be bigger than 1920x1080.
	* This function can only be called by the overlay's renderer process */
	virtual EVROverlayError SetOverlayFromFile(VROverlayHandle_t ulOverlayHandle, const char *pchFilePath);

	/** Get the native texture handle/device for an overlay you have created.
	* On windows this handle will be a ID3D11ShaderResourceView with a ID3D11Texture2D bound.
	*
	* The texture will always be sized to match the backing texture you supplied in SetOverlayTexture above.
	*
	* You MUST call ReleaseNativeOverlayHandle() with pNativeTextureHandle once you are done with this texture.
	*
	* pNativeTextureHandle is an OUTPUT, it will be a pointer to a ID3D11ShaderResourceView *.
	* pNativeTextureRef is an INPUT and should be a ID3D11Resource *. The device used by pNativeTextureRef will be used to bind pNativeTextureHandle.
	*/
	virtual EVROverlayError GetOverlayTexture(VROverlayHandle_t ulOverlayHandle, void **pNativeTextureHandle, void *pNativeTextureRef, uint32_t *pWidth, uint32_t *pHeight, uint32_t *pNativeFormat, ETextureType *pAPIType, EColorSpace *pColorSpace, VRTextureBounds_t *pTextureBounds);

	/** Release the pNativeTextureHandle provided from the GetOverlayTexture call, this allows the system to free the underlying GPU resources for this object,
	* so only do it once you stop rendering this texture.
	*/
	virtual EVROverlayError ReleaseNativeOverlayHandle(VROverlayHandle_t ulOverlayHandle, void *pNativeTextureHandle);

	/** Get the size of the overlay texture */
	virtual EVROverlayError GetOverlayTextureSize(VROverlayHandle_t ulOverlayHandle, uint32_t *pWidth, uint32_t *pHeight);

	// ----------------------------------------------
	// Dashboard Overlay Methods
	// ----------------------------------------------

	/** Creates a dashboard overlay and returns its handle */
	virtual EVROverlayError CreateDashboardOverlay(const char *pchOverlayKey, const char *pchOverlayFriendlyName, VROverlayHandle_t * pMainHandle, VROverlayHandle_t *pThumbnailHandle);

	/** Returns true if the dashboard is visible */
	virtual bool IsDashboardVisible();

	/** returns true if the dashboard is visible and the specified overlay is the active system Overlay */
	virtual bool IsActiveDashboardOverlay(VROverlayHandle_t ulOverlayHandle);

	/** Sets the dashboard overlay to only appear when the specified process ID has scene focus */
	virtual EVROverlayError SetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId);

	/** Gets the process ID that this dashboard overlay requires to have scene focus */
	virtual EVROverlayError GetDashboardOverlaySceneProcess(VROverlayHandle_t ulOverlayHandle, uint32_t *punProcessId);

	/** Shows the dashboard. */
	virtual void ShowDashboard(const char *pchOverlayToShow);

	/** Returns the tracked device that has the laser pointer in the dashboard */
	virtual TrackedDeviceIndex_t GetPrimaryDashboardDevice();

	// ---------------------------------------------
	// Keyboard methods
	// ---------------------------------------------

	/** Show the virtual keyboard to accept input **/
	virtual EVROverlayError ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue);

	virtual EVROverlayError ShowKeyboardForOverlay(VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue);

	/** Get the text that was entered into the text input **/
	virtual uint32_t GetKeyboardText(char *pchText, uint32_t cchText);

	/** Hide the virtual keyboard **/
	virtual void HideKeyboard();

	/** Set the position of the keyboard in world space **/
	virtual void SetKeyboardTransformAbsolute(ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToKeyboardTransform);

	/** Set the position of the keyboard in overlay space by telling it to avoid a rectangle in the overlay. Rectangle coords have (0,0) in the bottom left **/
	virtual void SetKeyboardPositionForOverlay(VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect);

	// ---------------------------------------------
	// Overlay input methods
	// ---------------------------------------------

	/** Sets a list of primitives to be used for controller ray intersection
	* typically the size of the underlying UI in pixels (not in world space). */
	// TODO structs
	//virtual EVROverlayError SetOverlayIntersectionMask(VROverlayHandle_t ulOverlayHandle, VROverlayIntersectionMaskPrimitive_t *pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize = sizeof(VROverlayIntersectionMaskPrimitive_t));

	virtual EVROverlayError GetOverlayFlags(VROverlayHandle_t ulOverlayHandle, uint32_t *pFlags);

	// ---------------------------------------------
	// Message box methods
	// ---------------------------------------------

	/** Show the message overlay. This will block and return you a result. **/
	virtual VRMessageOverlayResponse ShowMessageOverlay(const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text = nullptr, const char* pchButton2Text = nullptr, const char* pchButton3Text = nullptr);

	/** If the calling process owns the overlay and it's open, this will close it. **/
	virtual void CloseMessageOverlay();
};
