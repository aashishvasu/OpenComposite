#pragma once
#include "../BaseCommon.h" // TODO don't import from OCOVR, and remove the "../"
#include "../Misc/Keyboard/VRKeyboard.h" // TODO don't import from OCOVR, and remove the "../"
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <vector>

enum OOVR_VROverlayInputMethod {
	VROverlayInputMethod_None = 0, // No input events will be generated automatically for this overlay
	VROverlayInputMethod_Mouse = 1, // Tracked controllers will get mouse events automatically
	VROverlayInputMethod_DualAnalog = 2, // Analog inputs from tracked controllers are turned into DualAnalog events
};

/** Allows the caller to figure out which overlay transform getter to call. */
enum OOVR_VROverlayTransformType {
	VROverlayTransform_Absolute = 0,
	VROverlayTransform_TrackedDeviceRelative = 1,
	VROverlayTransform_SystemOverlay = 2,
	VROverlayTransform_TrackedComponent = 3,
};

/** Overlay control settings */
enum OOVR_VROverlayFlags {
	VROverlayFlags_None = 0,

	// The following only take effect when rendered using the high quality render path (see SetHighQualityOverlay).
	VROverlayFlags_Curved = 1,
	VROverlayFlags_RGSS4X = 2,

	// Set this flag on a dashboard overlay to prevent a tab from showing up for that overlay
	VROverlayFlags_NoDashboardTab = 3,

	// Set this flag on a dashboard that is able to deal with gamepad focus events
	VROverlayFlags_AcceptsGamepadEvents = 4,

	// Indicates that the overlay should dim/brighten to show gamepad focus
	VROverlayFlags_ShowGamepadFocus = 5,

	// When in VROverlayInputMethod_Mouse you can optionally enable sending VRScroll_t
	VROverlayFlags_SendVRScrollEvents = 6,
	VROverlayFlags_SendVRTouchpadEvents = 7,

	// If set this will render a vertical scroll wheel on the primary controller,
	//  only needed if not using VROverlayFlags_SendVRScrollEvents but you still want to represent a scroll wheel
	VROverlayFlags_ShowTouchPadScrollWheel = 8,

	// If this is set ownership and render access to the overlay are transferred
	// to the new scene process on a call to IVRApplications::LaunchInternalProcess
	VROverlayFlags_TransferOwnershipToInternalProcess = 9,

	// If set, renders 50% of the texture in each eye, side by side
	VROverlayFlags_SideBySide_Parallel = 10, // Texture is left/right
	VROverlayFlags_SideBySide_Crossed = 11, // Texture is crossed and right/left

	VROverlayFlags_Panorama = 12, // Texture is a panorama
	VROverlayFlags_StereoPanorama = 13, // Texture is a stereo panorama

	// If this is set on an overlay owned by the scene application that overlay
	// will be sorted with the "Other" overlays on top of all other scene overlays
	VROverlayFlags_SortWithNonSceneOverlays = 14,

	// If set, the overlay will be shown in the dashboard, otherwise it will be hidden.
	VROverlayFlags_VisibleInDashboard = 15,
};

enum OOVR_VRMessageOverlayResponse {
	VRMessageOverlayResponse_ButtonPress_0 = 0,
	VRMessageOverlayResponse_ButtonPress_1 = 1,
	VRMessageOverlayResponse_ButtonPress_2 = 2,
	VRMessageOverlayResponse_ButtonPress_3 = 3,
	VRMessageOverlayResponse_CouldntFindSystemOverlay = 4,
	VRMessageOverlayResponse_CouldntFindOrCreateClientOverlay = 5,
	VRMessageOverlayResponse_ApplicationQuit = 6
};

struct OOVR_VROverlayIntersectionParams_t {
	vr::HmdVector3_t vSource;
	vr::HmdVector3_t vDirection;
	vr::ETrackingUniverseOrigin eOrigin;
};

struct OOVR_VROverlayIntersectionResults_t {
	vr::HmdVector3_t vPoint;
	vr::HmdVector3_t vNormal;
	vr::HmdVector2_t vUVs;
	float fDistance;
};

// Input modes for the Big Picture gamepad text entry
enum OOVR_EGamepadTextInputMode {
	k_EGamepadTextInputModeNormal = 0,
	k_EGamepadTextInputModePassword = 1,
	k_EGamepadTextInputModeSubmit = 2,
};

// Controls number of allowed lines for the Big Picture gamepad text entry
enum OOVR_EGamepadTextInputLineMode {
	k_EGamepadTextInputLineModeSingleLine = 0,
	k_EGamepadTextInputLineModeMultipleLines = 1
};

/** Directions for changing focus between overlays with the gamepad */
enum OOVR_EOverlayDirection {
	OverlayDirection_Up = 0,
	OverlayDirection_Down = 1,
	OverlayDirection_Left = 2,
	OverlayDirection_Right = 3,

	OverlayDirection_Count = 4,
};

enum OOVR_EVROverlayIntersectionMaskPrimitiveType {
	OverlayIntersectionPrimitiveType_Rectangle,
	OverlayIntersectionPrimitiveType_Circle,
};

struct OOVR_IntersectionMaskRectangle_t {
	float m_flTopLeftX;
	float m_flTopLeftY;
	float m_flWidth;
	float m_flHeight;
};

struct OOVR_IntersectionMaskCircle_t {
	float m_flCenterX;
	float m_flCenterY;
	float m_flRadius;
};

typedef union {
	OOVR_IntersectionMaskRectangle_t m_Rectangle;
	OOVR_IntersectionMaskCircle_t m_Circle;
} OOVR_VROverlayIntersectionMaskPrimitive_Data_t;

struct OOVR_VROverlayIntersectionMaskPrimitive_t {
	OOVR_EVROverlayIntersectionMaskPrimitiveType m_nPrimitiveType;
	OOVR_VROverlayIntersectionMaskPrimitive_Data_t m_Primitive;
};

/** Defines the project used in an overlay that is using SetOverlayTransformProjection */
struct OOVR_VROverlayProjection_t {
	/** Tangent of the sides of the frustum */
	float fLeft;
	float fRight;
	float fTop;
	float fBottom;
};

// Avoid importing LibOVR just for this
struct ovrLayerHeader_;

class BaseOverlay {
private:
	// These enums are ints
	typedef OOVR_VROverlayFlags VROverlayFlags;
	typedef OOVR_VROverlayTransformType VROverlayTransformType;
	typedef OOVR_VROverlayInputMethod VROverlayInputMethod;
	typedef OOVR_EOverlayDirection EOverlayDirection;
	typedef OOVR_EGamepadTextInputMode EGamepadTextInputMode;
	typedef OOVR_EGamepadTextInputLineMode EGamepadTextInputLineMode;
	typedef OOVR_VRMessageOverlayResponse VRMessageOverlayResponse;

	class OverlayData;

	// Name-to-overlay mapping
	std::map<std::string, OverlayData*> overlays;

	// Set of overlay pointers - use these to ensure the overlays are valid, and
	// if games pass in some random value (*COUGH* Boneworks *COUGH) we can deal
	// with it, rather than crashing everything.
	std::set<OverlayData*> validOverlays;

	// This doesn't do a whole lot, since OOVR does this for every overlay
	vr::VROverlayHandle_t highQualityOverlay;

	// List of layers, with the first being reserved for the main scene
	std::vector<XrCompositionLayerBaseHeader*> layerHeaders;

	// Virtual Keyboard
	std::unique_ptr<VRKeyboard> keyboard;

	// Cached copy of the keyboard contents, available after it is closed
	std::string keyboardCache;

	// True if we're modifying the input in any way
	bool usingInput;

	virtual vr::EVROverlayError ShowKeyboardWithDispatch(
	    EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode,
	    const char* pchDescription, uint32_t unCharMax, const char* pchExistingText,
	    bool bUseMinimalMode, uint64_t uUserValue,
	    VRKeyboard::eventDispatch_t eventDispatch);

public:
	// Destructor, since we have a map of pointers
	~BaseOverlay();

	// Builds the collection of layers to be submitted to LibOVR
	int _BuildLayers(XrCompositionLayerBaseHeader* sceneLayer, XrCompositionLayerBaseHeader const* const*& result);

	// If an overlay needs input, this grabs the input and returns whether the input should preceed to the application
	bool _HandleOverlayInput(vr::EVREye role, vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t state);

	// ---------------------------------------------
	// Overlay management methods
	// ---------------------------------------------

	/** Finds an existing overlay with the specified key. */
	virtual vr::EVROverlayError FindOverlay(const char* pchOverlayKey, vr::VROverlayHandle_t* pOverlayHandle);

	/** Creates a new named overlay. All overlays start hidden and with default settings. */
	virtual vr::EVROverlayError CreateOverlay(const char* pchOverlayKey, const char* pchOverlayName, vr::VROverlayHandle_t* pOverlayHandle);

	/** Destroys the specified overlay. When an application calls VR_Shutdown all overlays created by that app are
	 * automatically destroyed. */
	virtual vr::EVROverlayError DestroyOverlay(vr::VROverlayHandle_t ulOverlayHandle);

	/** Specify which overlay to use the high quality render path.  This overlay will be composited in during the distortion pass which
	 * results in it drawing on top of everything else, but also at a higher quality as it samples the source texture directly rather than
	 * rasterizing into each eye's render texture first.  Because if this, only one of these is supported at any given time.  It is most useful
	 * for overlays that are expected to take up most of the user's view (e.g. streaming video).
	 * This mode does not support mouse input to your overlay. */
	virtual vr::EVROverlayError SetHighQualityOverlay(vr::VROverlayHandle_t ulOverlayHandle);

	/** Returns the overlay handle of the current overlay being rendered using the single high quality overlay render path.
	 * Otherwise it will return k_ulOverlayHandleInvalid. */
	virtual vr::VROverlayHandle_t GetHighQualityOverlay();

	/** Fills the provided buffer with the string key of the overlay. Returns the size of buffer required to store the key, including
	 * the terminating null character. k_unVROverlayMaxKeyLength will be enough bytes to fit the string. */
	virtual uint32_t GetOverlayKey(vr::VROverlayHandle_t ulOverlayHandle, char* pchValue, uint32_t unBufferSize, vr::EVROverlayError* pError = 0L);

	/** Fills the provided buffer with the friendly name of the overlay. Returns the size of buffer required to store the key, including
	 * the terminating null character. k_unVROverlayMaxNameLength will be enough bytes to fit the string. */
	virtual uint32_t GetOverlayName(vr::VROverlayHandle_t ulOverlayHandle, char* pchValue, uint32_t unBufferSize, vr::EVROverlayError* pError = 0L);

	/** set the name to use for this overlay */
	virtual vr::EVROverlayError SetOverlayName(vr::VROverlayHandle_t ulOverlayHandle, const char* pchName);

	/** Gets the raw image data from an overlay. Overlay image data is always returned as RGBA data, 4 bytes per pixel. If the buffer is not large enough, width and height
	 * will be set and VROverlayError_ArrayTooSmall is returned. */
	virtual vr::EVROverlayError GetOverlayImageData(vr::VROverlayHandle_t ulOverlayHandle, void* pvBuffer, uint32_t unBufferSize, uint32_t* punWidth, uint32_t* punHeight);

	/** returns a string that corresponds with the specified overlay error. The string will be the name
	 * of the error enum value for all valid error codes */
	virtual const char* GetOverlayErrorNameFromEnum(vr::EVROverlayError error);

	// ---------------------------------------------
	// Overlay rendering methods
	// ---------------------------------------------

	/** Sets the pid that is allowed to render to this overlay (the creator pid is always allow to render),
	 *	by default this is the pid of the process that made the overlay */
	virtual vr::EVROverlayError SetOverlayRenderingPid(vr::VROverlayHandle_t ulOverlayHandle, uint32_t unPID);

	/** Gets the pid that is allowed to render to this overlay */
	virtual uint32_t GetOverlayRenderingPid(vr::VROverlayHandle_t ulOverlayHandle);

	/** Specify flag setting for a given overlay */
	virtual vr::EVROverlayError SetOverlayFlag(vr::VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled);

	/** Sets flag setting for a given overlay */
	virtual vr::EVROverlayError GetOverlayFlag(vr::VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool* pbEnabled);

	/** Sets the color tint of the overlay quad. Use 0.0 to 1.0 per channel. */
	virtual vr::EVROverlayError SetOverlayColor(vr::VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue);

	/** Gets the color tint of the overlay quad. */
	virtual vr::EVROverlayError GetOverlayColor(vr::VROverlayHandle_t ulOverlayHandle, float* pfRed, float* pfGreen, float* pfBlue);

	/** Sets the alpha of the overlay quad. Use 1.0 for 100 percent opacity to 0.0 for 0 percent opacity. */
	virtual vr::EVROverlayError SetOverlayAlpha(vr::VROverlayHandle_t ulOverlayHandle, float fAlpha);

	/** Gets the alpha of the overlay quad. By default overlays are rendering at 100 percent alpha (1.0). */
	virtual vr::EVROverlayError GetOverlayAlpha(vr::VROverlayHandle_t ulOverlayHandle, float* pfAlpha);

	/** Sets the aspect ratio of the texels in the overlay. 1.0 means the texels are square. 2.0 means the texels
	 * are twice as wide as they are tall. Defaults to 1.0. */
	virtual vr::EVROverlayError SetOverlayTexelAspect(vr::VROverlayHandle_t ulOverlayHandle, float fTexelAspect);

	/** Gets the aspect ratio of the texels in the overlay. Defaults to 1.0 */
	virtual vr::EVROverlayError GetOverlayTexelAspect(vr::VROverlayHandle_t ulOverlayHandle, float* pfTexelAspect);

	/** Sets the rendering sort order for the overlay. Overlays are rendered this order:
	 *      Overlays owned by the scene application
	 *      Overlays owned by some other application
	 *
	 *	Within a category overlays are rendered lowest sort order to highest sort order. Overlays with the same
	 *	sort order are rendered back to front base on distance from the HMD.
	 *
	 *	Sort order defaults to 0. */
	virtual vr::EVROverlayError SetOverlaySortOrder(vr::VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder);

	/** Gets the sort order of the overlay. See SetOverlaySortOrder for how this works. */
	virtual vr::EVROverlayError GetOverlaySortOrder(vr::VROverlayHandle_t ulOverlayHandle, uint32_t* punSortOrder);

	/** Sets the width of the overlay quad in meters. By default overlays are rendered on a quad that is 1 meter across */
	virtual vr::EVROverlayError SetOverlayWidthInMeters(vr::VROverlayHandle_t ulOverlayHandle, float fWidthInMeters);

	/** Returns the width of the overlay quad in meters. By default overlays are rendered on a quad that is 1 meter across */
	virtual vr::EVROverlayError GetOverlayWidthInMeters(vr::VROverlayHandle_t ulOverlayHandle, float* pfWidthInMeters);

	/** Use to draw overlay as a curved surface. Curvature is a percentage from (0..1] where 1 is a fully closed cylinder.
	 * For a specific radius, curvature can be computed as: overlay.width / (2 PI r). */
	virtual vr::EVROverlayError SetOverlayCurvature(vr::VROverlayHandle_t ulOverlayHandle, float fCurvature);

	/** Returns the curvature of the overlay as a percentage from (0..1] where 1 is a fully closed cylinder. */
	virtual vr::EVROverlayError GetOverlayCurvature(vr::VROverlayHandle_t ulOverlayHandle, float* pfCurvature);

	/** For high-quality curved overlays only, sets the distance range in meters from the overlay used to automatically curve
	 * the surface around the viewer.  Min is distance is when the surface will be most curved.  Max is when least curved. */
	virtual vr::EVROverlayError SetOverlayAutoCurveDistanceRangeInMeters(vr::VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters);

	/** For high-quality curved overlays only, gets the distance range in meters from the overlay used to automatically curve
	 * the surface around the viewer.  Min is distance is when the surface will be most curved.  Max is when least curved. */
	virtual vr::EVROverlayError GetOverlayAutoCurveDistanceRangeInMeters(vr::VROverlayHandle_t ulOverlayHandle, float* pfMinDistanceInMeters, float* pfMaxDistanceInMeters);

	/** Sets the colorspace the overlay texture's data is in.  Defaults to 'auto'.
	 * If the texture needs to be resolved, you should call SetOverlayTexture with the appropriate colorspace instead. */
	virtual vr::EVROverlayError SetOverlayTextureColorSpace(vr::VROverlayHandle_t ulOverlayHandle, vr::EColorSpace eTextureColorSpace);

	/** Gets the overlay's current colorspace setting. */
	virtual vr::EVROverlayError GetOverlayTextureColorSpace(vr::VROverlayHandle_t ulOverlayHandle, vr::EColorSpace* peTextureColorSpace);

	/** Sets the part of the texture to use for the overlay. UV Min is the upper left corner and UV Max is the lower right corner. */
	virtual vr::EVROverlayError SetOverlayTextureBounds(vr::VROverlayHandle_t ulOverlayHandle, const vr::VRTextureBounds_t* pOverlayTextureBounds);

	/** Gets the part of the texture to use for the overlay. UV Min is the upper left corner and UV Max is the lower right corner. */
	virtual vr::EVROverlayError GetOverlayTextureBounds(vr::VROverlayHandle_t ulOverlayHandle, vr::VRTextureBounds_t* pOverlayTextureBounds);

	/** Gets render model to draw behind this overlay */
	virtual uint32_t GetOverlayRenderModel(vr::VROverlayHandle_t ulOverlayHandle, char* pchValue, uint32_t unBufferSize, vr::HmdColor_t* pColor, vr::EVROverlayError* pError);

	/** Sets render model to draw behind this overlay and the vertex color to use, pass null for pColor to match the overlays vertex color.
	The model is scaled by the same amount as the overlay, with a default of 1m. */
	virtual vr::EVROverlayError SetOverlayRenderModel(vr::VROverlayHandle_t ulOverlayHandle, const char* pchRenderModel, const vr::HmdColor_t* pColor);

	/** Returns the transform type of this overlay. */
	virtual vr::EVROverlayError GetOverlayTransformType(vr::VROverlayHandle_t ulOverlayHandle, VROverlayTransformType* peTransformType);

	/** Sets the transform to absolute tracking origin. */
	virtual vr::EVROverlayError SetOverlayTransformAbsolute(vr::VROverlayHandle_t ulOverlayHandle, vr::ETrackingUniverseOrigin eTrackingOrigin, const vr::HmdMatrix34_t* pmatTrackingOriginToOverlayTransform);

	/** Gets the transform if it is absolute. Returns an error if the transform is some other type. */
	virtual vr::EVROverlayError GetOverlayTransformAbsolute(vr::VROverlayHandle_t ulOverlayHandle, vr::ETrackingUniverseOrigin* peTrackingOrigin, vr::HmdMatrix34_t* pmatTrackingOriginToOverlayTransform);

	/** Sets the transform to relative to the transform of the specified tracked device. */
	virtual vr::EVROverlayError SetOverlayTransformTrackedDeviceRelative(vr::VROverlayHandle_t ulOverlayHandle, vr::TrackedDeviceIndex_t unTrackedDevice, const vr::HmdMatrix34_t* pmatTrackedDeviceToOverlayTransform);

	/** Gets the transform if it is relative to a tracked device. Returns an error if the transform is some other type. */
	virtual vr::EVROverlayError GetOverlayTransformTrackedDeviceRelative(vr::VROverlayHandle_t ulOverlayHandle, vr::TrackedDeviceIndex_t* punTrackedDevice, vr::HmdMatrix34_t* pmatTrackedDeviceToOverlayTransform);

	/** Sets the transform to draw the overlay on a rendermodel component mesh instead of a quad. This will only draw when the system is
	 * drawing the device. Overlays with this transform type cannot receive mouse events. */
	virtual vr::EVROverlayError SetOverlayTransformTrackedDeviceComponent(vr::VROverlayHandle_t ulOverlayHandle, vr::TrackedDeviceIndex_t unDeviceIndex, const char* pchComponentName);

	/** Gets the transform information when the overlay is rendering on a component. */
	virtual vr::EVROverlayError GetOverlayTransformTrackedDeviceComponent(vr::VROverlayHandle_t ulOverlayHandle, vr::TrackedDeviceIndex_t* punDeviceIndex, char* pchComponentName, uint32_t unComponentNameSize);

	/** Gets the transform if it is relative to another overlay. Returns an error if the transform is some other type. */
	virtual vr::EVROverlayError GetOverlayTransformOverlayRelative(vr::VROverlayHandle_t ulOverlayHandle, vr::VROverlayHandle_t* ulOverlayHandleParent, vr::HmdMatrix34_t* pmatParentOverlayToOverlayTransform);

	/** Sets the transform to relative to the transform of the specified overlay. This overlays visibility will also track the parents visibility */
	virtual vr::EVROverlayError SetOverlayTransformOverlayRelative(vr::VROverlayHandle_t ulOverlayHandle, vr::VROverlayHandle_t ulOverlayHandleParent, const vr::HmdMatrix34_t* pmatParentOverlayToOverlayTransform);

	/** Sets the hotspot for the specified overlay when that overlay is used as a cursor. These are in texture space with 0,0 in the upper left corner of
	 * the texture and 1,1 in the lower right corner of the texture. */
	virtual vr::EVROverlayError SetOverlayTransformCursor(vr::VROverlayHandle_t ulCursorOverlayHandle, const vr::HmdVector2_t* pvHotspot);

	/** Gets cursor hotspot/transform for the specified overlay */
	virtual vr::EVROverlayError GetOverlayTransformCursor(vr::VROverlayHandle_t ulOverlayHandle, vr::HmdVector2_t* pvHotspot);

	/** Sets the overlay as a projection overlay */
	virtual vr::EVROverlayError SetOverlayTransformProjection(vr::VROverlayHandle_t ulOverlayHandle,
	    vr::ETrackingUniverseOrigin eTrackingOrigin, const vr::HmdMatrix34_t* pmatTrackingOriginToOverlayTransform,
	    const OOVR_VROverlayProjection_t* pProjection, vr::EVREye eEye);

	/** Shows the VR overlay.  For dashboard overlays, only the Dashboard Manager is allowed to call this. */
	virtual vr::EVROverlayError ShowOverlay(vr::VROverlayHandle_t ulOverlayHandle);

	/** Hides the VR overlay.  For dashboard overlays, only the Dashboard Manager is allowed to call this. */
	virtual vr::EVROverlayError HideOverlay(vr::VROverlayHandle_t ulOverlayHandle);

	/** Returns true if the overlay is visible. */
	virtual bool IsOverlayVisible(vr::VROverlayHandle_t ulOverlayHandle);

	/** Get the transform in 3d space associated with a specific 2d point in the overlay's coordinate space (where 0,0 is the lower left). -Z points out of the overlay */
	virtual vr::EVROverlayError GetTransformForOverlayCoordinates(vr::VROverlayHandle_t ulOverlayHandle, vr::ETrackingUniverseOrigin eTrackingOrigin, vr::HmdVector2_t coordinatesInOverlay, vr::HmdMatrix34_t* pmatTransform);

	// ---------------------------------------------
	// Overlay input methods
	// ---------------------------------------------

	/** Returns true and fills the event with the next event on the overlay's event queue, if there is one.
	 * If there are no events this method returns false. uncbVREvent should be the size in bytes of the VREvent_t struct */
	virtual bool PollNextOverlayEvent(vr::VROverlayHandle_t ulOverlayHandle, vr::VREvent_t* pEvent, uint32_t uncbVREvent);

	/** Returns the current input settings for the specified overlay. */
	virtual vr::EVROverlayError GetOverlayInputMethod(vr::VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod* peInputMethod);

	/** Sets the input settings for the specified overlay. */
	virtual vr::EVROverlayError SetOverlayInputMethod(vr::VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod);

	/** Gets the mouse scaling factor that is used for mouse events. The actual texture may be a different size, but this is
	 * typically the size of the underlying UI in pixels. */
	virtual vr::EVROverlayError GetOverlayMouseScale(vr::VROverlayHandle_t ulOverlayHandle, vr::HmdVector2_t* pvecMouseScale);

	/** Sets the mouse scaling factor that is used for mouse events. The actual texture may be a different size, but this is
	 * typically the size of the underlying UI in pixels (not in world space). */
	virtual vr::EVROverlayError SetOverlayMouseScale(vr::VROverlayHandle_t ulOverlayHandle, const vr::HmdVector2_t* pvecMouseScale);

	/** Computes the overlay-space pixel coordinates of where the ray intersects the overlay with the
	 * specified settings. Returns false if there is no intersection. */
	virtual bool ComputeOverlayIntersection(vr::VROverlayHandle_t ulOverlayHandle, const OOVR_VROverlayIntersectionParams_t* pParams, OOVR_VROverlayIntersectionResults_t* pResults);

	/** Processes mouse input from the specified controller as though it were a mouse pointed at a compositor overlay with the
	 * specified settings. The controller is treated like a laser pointer on the -z axis. The point where the laser pointer would
	 * intersect with the overlay is the mouse position, the trigger is left mouse, and the track pad is right mouse.
	 *
	 * Return true if the controller is pointed at the overlay and an event was generated. */
	virtual bool HandleControllerOverlayInteractionAsMouse(vr::VROverlayHandle_t ulOverlayHandle, vr::TrackedDeviceIndex_t unControllerDeviceIndex);

	/** Returns true if the specified overlay is the hover target. An overlay is the hover target when it is the last overlay "moused over"
	 * by the virtual mouse pointer */
	virtual bool IsHoverTargetOverlay(vr::VROverlayHandle_t ulOverlayHandle);

	/** Returns the current Gamepad focus overlay */
	virtual vr::VROverlayHandle_t GetGamepadFocusOverlay();

	/** Sets the current Gamepad focus overlay */
	virtual vr::EVROverlayError SetGamepadFocusOverlay(vr::VROverlayHandle_t ulNewFocusOverlay);

	/** Sets an overlay's neighbor. This will also set the neighbor of the "to" overlay
	 * to point back to the "from" overlay. If an overlay's neighbor is set to invalid both
	 * ends will be cleared */
	virtual vr::EVROverlayError SetOverlayNeighbor(EOverlayDirection eDirection, vr::VROverlayHandle_t ulFrom, vr::VROverlayHandle_t ulTo);

	/** Changes the Gamepad focus from one overlay to one of its neighbors. Returns VROverlayError_NoNeighbor if there is no
	 * neighbor in that direction */
	virtual vr::EVROverlayError MoveGamepadFocusToNeighbor(EOverlayDirection eDirection, vr::VROverlayHandle_t ulFrom);

	/** Sets the analog input to Dual Analog coordinate scale for the specified overlay. */
	virtual vr::EVROverlayError SetOverlayDualAnalogTransform(vr::VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const vr::HmdVector2_t& vCenter, float fRadius);

	/** Gets the analog input to Dual Analog coordinate scale for the specified overlay. */
	virtual vr::EVROverlayError GetOverlayDualAnalogTransform(vr::VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, vr::HmdVector2_t* pvCenter, float* pfRadius);

	/** Sets the analog input to Dual Analog coordinate scale for the specified overlay. */
	virtual vr::EVROverlayError SetOverlayDualAnalogTransform(vr::VROverlayHandle_t ulOverlay, EDualAnalogWhich eWhich, const vr::HmdVector2_t* pvCenter, float fRadius);

	/** Triggers a haptic event on the laser mouse controller for the specified overlay */
	virtual vr::EVROverlayError TriggerLaserMouseHapticVibration(vr::VROverlayHandle_t ulOverlayHandle, float fDurationSeconds, float fFrequency, float fAmplitude);

	/** Sets the cursor to use for the specified overlay. This will be drawn instead of the generic blob when the laser mouse is pointed at the specified overlay */
	virtual vr::EVROverlayError SetOverlayCursor(vr::VROverlayHandle_t ulOverlayHandle, vr::VROverlayHandle_t ulCursorHandle);

	/** Sets the override cursor position to use for this overlay in overlay mouse coordinates. This position will be used to draw the cursor
	 * instead of whatever the laser mouse cursor position is. */
	virtual vr::EVROverlayError SetOverlayCursorPositionOverride(vr::VROverlayHandle_t ulOverlayHandle, const vr::HmdVector2_t* pvCursor);

	/** Clears the override cursor position for this overlay */
	virtual vr::EVROverlayError ClearOverlayCursorPositionOverride(vr::VROverlayHandle_t ulOverlayHandle);

	// ---------------------------------------------
	// Overlay texture methods
	// ---------------------------------------------

	/** Texture to draw for the overlay. This function can only be called by the overlay's creator or renderer process (see SetOverlayRenderingPid) .
	 *
	 * OpenGL dirty state:
	 *	glBindTexture
	 */
	virtual vr::EVROverlayError SetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle, const vr::Texture_t* pTexture);

	/** Use this to tell the overlay system to release the texture set for this overlay. */
	virtual vr::EVROverlayError ClearOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle);

	/** Separate interface for providing the data as a stream of bytes, but there is an upper bound on data
	 * that can be sent. This function can only be called by the overlay's renderer process. */
	virtual vr::EVROverlayError SetOverlayRaw(vr::VROverlayHandle_t ulOverlayHandle, void* pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth);

	/** Separate interface for providing the image through a filename: can be png or jpg, and should not be bigger than 1920x1080.
	 * This function can only be called by the overlay's renderer process */
	virtual vr::EVROverlayError SetOverlayFromFile(vr::VROverlayHandle_t ulOverlayHandle, const char* pchFilePath);

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
	virtual vr::EVROverlayError GetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle, void** pNativeTextureHandle, void* pNativeTextureRef, uint32_t* pWidth, uint32_t* pHeight, uint32_t* pNativeFormat, vr::ETextureType* pAPIType, vr::EColorSpace* pColorSpace, vr::VRTextureBounds_t* pTextureBounds);

	/** Release the pNativeTextureHandle provided from the GetOverlayTexture call, this allows the system to free the underlying GPU resources for this object,
	 * so only do it once you stop rendering this texture.
	 */
	virtual vr::EVROverlayError ReleaseNativeOverlayHandle(vr::VROverlayHandle_t ulOverlayHandle, void* pNativeTextureHandle);

	/** Get the size of the overlay texture */
	virtual vr::EVROverlayError GetOverlayTextureSize(vr::VROverlayHandle_t ulOverlayHandle, uint32_t* pWidth, uint32_t* pHeight);

	// ----------------------------------------------
	// Dashboard Overlay Methods
	// ----------------------------------------------

	/** Creates a dashboard overlay and returns its handle */
	virtual vr::EVROverlayError CreateDashboardOverlay(const char* pchOverlayKey, const char* pchOverlayFriendlyName, vr::VROverlayHandle_t* pMainHandle, vr::VROverlayHandle_t* pThumbnailHandle);

	/** Returns true if the dashboard is visible */
	virtual bool IsDashboardVisible();

	/** returns true if the dashboard is visible and the specified overlay is the active system Overlay */
	virtual bool IsActiveDashboardOverlay(vr::VROverlayHandle_t ulOverlayHandle);

	/** Sets the dashboard overlay to only appear when the specified process ID has scene focus */
	virtual vr::EVROverlayError SetDashboardOverlaySceneProcess(vr::VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId);

	/** Gets the process ID that this dashboard overlay requires to have scene focus */
	virtual vr::EVROverlayError GetDashboardOverlaySceneProcess(vr::VROverlayHandle_t ulOverlayHandle, uint32_t* punProcessId);

	/** Shows the dashboard. */
	virtual void ShowDashboard(const char* pchOverlayToShow);

	/** Returns the tracked device that has the laser pointer in the dashboard */
	virtual vr::TrackedDeviceIndex_t GetPrimaryDashboardDevice();

	// ---------------------------------------------
	// Keyboard methods
	// ---------------------------------------------

	/** Show the virtual keyboard to accept input **/
	virtual vr::EVROverlayError ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char* pchDescription, uint32_t unCharMax, const char* pchExistingText, bool bUseMinimalMode, uint64_t uUserValue);

	/** Placeholder method for submitting a KeyboardDone event when asked to show the keyboard since it is not implemented yet. **/
	virtual void SubmitPlaceholderKeyboardEvent(vr::EVREventType ev, VRKeyboard::eventDispatch_t eventDispatch, uint64_t userValue);

	/**
	 * Show the virtual keyboard to accept input. In most cases, you should pass KeyboardFlag_Modal to enable modal overlay
	 * behavior on the keyboard itself. See EKeyboardFlags for more.
	 */
	virtual vr::EVROverlayError ShowKeyboard(EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, uint32_t unFlags,
	    const char* pchDescription, uint32_t unCharMax, const char* pchExistingText, uint64_t uUserValue);

	virtual vr::EVROverlayError ShowKeyboardForOverlay(vr::VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char* pchDescription, uint32_t unCharMax, const char* pchExistingText, bool bUseMinimalMode, uint64_t uUserValue);

	/**
	 * Show the virtual keyboard to accept input for an overlay. In most cases, you should pass KeyboardFlag_Modal to enable modal
	 * overlay behavior on the keyboard itself. See EKeyboardFlags for more.
	 */
	virtual vr::EVROverlayError ShowKeyboardForOverlay(vr::VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode,
	    EGamepadTextInputLineMode eLineInputMode, uint32_t unFlags, const char* pchDescription, uint32_t unCharMax,
	    const char* pchExistingText, uint64_t uUserValue);

	/** Get the text that was entered into the text input **/
	virtual uint32_t GetKeyboardText(char* pchText, uint32_t cchText);

	/** Hide the virtual keyboard **/
	virtual void HideKeyboard();

	/** Set the position of the keyboard in world space **/
	virtual void SetKeyboardTransformAbsolute(vr::ETrackingUniverseOrigin eTrackingOrigin, const vr::HmdMatrix34_t* pmatTrackingOriginToKeyboardTransform);

	/** Set the position of the keyboard in overlay space by telling it to avoid a rectangle in the overlay. Rectangle coords have (0,0) in the bottom left **/
	virtual void SetKeyboardPositionForOverlay(vr::VROverlayHandle_t ulOverlayHandle, vr::HmdRect2_t avoidRect);

	// ---------------------------------------------
	// Overlay input methods
	// ---------------------------------------------

	/** Sets a list of primitives to be used for controller ray intersection
	 * typically the size of the underlying UI in pixels (not in world space). */
	virtual vr::EVROverlayError SetOverlayIntersectionMask(vr::VROverlayHandle_t ulOverlayHandle, OOVR_VROverlayIntersectionMaskPrimitive_t* pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize = sizeof(OOVR_VROverlayIntersectionMaskPrimitive_t));

	virtual vr::EVROverlayError GetOverlayFlags(vr::VROverlayHandle_t ulOverlayHandle, uint32_t* pFlags);

	// ---------------------------------------------
	// Message box methods
	// ---------------------------------------------

	/** Show the message overlay. This will block and return you a result. **/
	virtual VRMessageOverlayResponse ShowMessageOverlay(const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text = nullptr, const char* pchButton2Text = nullptr, const char* pchButton3Text = nullptr);

	/** If the calling process owns the overlay and it's open, this will close it. **/
	virtual void CloseMessageOverlay();

	virtual vr::EVROverlayError SetOverlayPreCurvePitch(vr::VROverlayHandle_t ulOverlayHandle, float fRadians);
	virtual vr::EVROverlayError GetOverlayPreCurvePitch(vr::VROverlayHandle_t ulOverlayHandle, float* pfRadians);
	virtual vr::EVROverlayError WaitFrameSync(uint32_t nTimeoutMs);
};
