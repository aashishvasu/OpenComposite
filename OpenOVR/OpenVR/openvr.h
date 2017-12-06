#pragma once

// openvr.h
//========= Copyright Valve Corporation ============//
// Dynamically generated file. Do not modify this file directly.

#ifndef _OPENVR_API
#define _OPENVR_API

#include "vrtypes.h"
#include "vrannotation.h"
#include "ivrsystem.h"
#include "ivrapplications.h"
#include "ivrchaperone.h"
#include "ivrcompositor.h"

// ivrnotifications.h
namespace vr
{

#pragma pack( push, 8 )

// Used for passing graphic data
struct NotificationBitmap_t
{
	NotificationBitmap_t()
		: m_pImageData( nullptr )
		, m_nWidth( 0 )
		, m_nHeight( 0 )
		, m_nBytesPerPixel( 0 )
	{
	};

	void *m_pImageData;
	int32_t m_nWidth;
	int32_t m_nHeight;
	int32_t m_nBytesPerPixel;
};


/** Be aware that the notification type is used as 'priority' to pick the next notification */
enum EVRNotificationType
{
	/** Transient notifications are automatically hidden after a period of time set by the user. 
	* They are used for things like information and chat messages that do not require user interaction. */
	EVRNotificationType_Transient = 0,

	/** Persistent notifications are shown to the user until they are hidden by calling RemoveNotification().
	* They are used for things like phone calls and alarms that require user interaction. */
	EVRNotificationType_Persistent = 1,

	/** System notifications are shown no matter what. It is expected, that the ulUserValue is used as ID.
	 * If there is already a system notification in the queue with that ID it is not accepted into the queue
	 * to prevent spamming with system notification */
	EVRNotificationType_Transient_SystemWithUserValue = 2,
};

enum EVRNotificationStyle
{
	/** Creates a notification with minimal external styling. */
	EVRNotificationStyle_None = 0,

	/** Used for notifications about overlay-level status. In Steam this is used for events like downloads completing. */
	EVRNotificationStyle_Application = 100,

	/** Used for notifications about contacts that are unknown or not available. In Steam this is used for friend invitations and offline friends. */
	EVRNotificationStyle_Contact_Disabled = 200,

	/** Used for notifications about contacts that are available but inactive. In Steam this is used for friends that are online but not playing a game. */
	EVRNotificationStyle_Contact_Enabled = 201,

	/** Used for notifications about contacts that are available and active. In Steam this is used for friends that are online and currently running a game. */
	EVRNotificationStyle_Contact_Active = 202,
};

static const uint32_t k_unNotificationTextMaxSize = 256;

typedef uint32_t VRNotificationId;



#pragma pack( pop )

/** Allows notification sources to interact with the VR system
	This current interface is not yet implemented. Do not use yet. */
class IVRNotifications
{
public:
	/** Create a notification and enqueue it to be shown to the user.
	* An overlay handle is required to create a notification, as otherwise it would be impossible for a user to act on it.
	* To create a two-line notification, use a line break ('\n') to split the text into two lines.
	* The pImage argument may be NULL, in which case the specified overlay's icon will be used instead. */
	virtual EVRNotificationError CreateNotification( VROverlayHandle_t ulOverlayHandle, uint64_t ulUserValue, EVRNotificationType type, const char *pchText, EVRNotificationStyle style, const NotificationBitmap_t *pImage, /* out */ VRNotificationId *pNotificationId ) = 0;

	/** Destroy a notification, hiding it first if it currently shown to the user. */
	virtual EVRNotificationError RemoveNotification( VRNotificationId notificationId ) = 0;

};

static const char * const IVRNotifications_Version = "IVRNotifications_002";

} // namespace vr



// ivroverlay.h
namespace vr
{

	/** The maximum length of an overlay key in bytes, counting the terminating null character. */
	static const uint32_t k_unVROverlayMaxKeyLength = 128;

	/** The maximum length of an overlay name in bytes, counting the terminating null character. */
	static const uint32_t k_unVROverlayMaxNameLength = 128;

	/** The maximum number of overlays that can exist in the system at one time. */
	static const uint32_t k_unMaxOverlayCount = 64;

	/** The maximum number of overlay intersection mask primitives per overlay */
	static const uint32_t k_unMaxOverlayIntersectionMaskPrimitivesCount = 32;

	/** Types of input supported by VR Overlays */
	enum VROverlayInputMethod
	{
		VROverlayInputMethod_None		= 0, // No input events will be generated automatically for this overlay
		VROverlayInputMethod_Mouse		= 1, // Tracked controllers will get mouse events automatically
	};

	/** Allows the caller to figure out which overlay transform getter to call. */
	enum VROverlayTransformType
	{
		VROverlayTransform_Absolute					= 0,
		VROverlayTransform_TrackedDeviceRelative	= 1,
		VROverlayTransform_SystemOverlay			= 2,
		VROverlayTransform_TrackedComponent 		= 3,
	};

	/** Overlay control settings */
	enum VROverlayFlags
	{
		VROverlayFlags_None			= 0,

		// The following only take effect when rendered using the high quality render path (see SetHighQualityOverlay).
		VROverlayFlags_Curved		= 1,
		VROverlayFlags_RGSS4X		= 2,

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

	enum VRMessageOverlayResponse
	{
		VRMessageOverlayResponse_ButtonPress_0 = 0,
		VRMessageOverlayResponse_ButtonPress_1 = 1,
		VRMessageOverlayResponse_ButtonPress_2 = 2,
		VRMessageOverlayResponse_ButtonPress_3 = 3,
		VRMessageOverlayResponse_CouldntFindSystemOverlay = 4,
		VRMessageOverlayResponse_CouldntFindOrCreateClientOverlay= 5,
		VRMessageOverlayResponse_ApplicationQuit = 6
	};

	struct VROverlayIntersectionParams_t
	{
		HmdVector3_t vSource;
		HmdVector3_t vDirection;
		ETrackingUniverseOrigin eOrigin;
	};

	struct VROverlayIntersectionResults_t
	{
		HmdVector3_t vPoint;
		HmdVector3_t vNormal;
		HmdVector2_t vUVs;
		float fDistance;
	};

	// Input modes for the Big Picture gamepad text entry
	enum EGamepadTextInputMode
	{
		k_EGamepadTextInputModeNormal = 0,
		k_EGamepadTextInputModePassword = 1,
		k_EGamepadTextInputModeSubmit = 2,
	};

	// Controls number of allowed lines for the Big Picture gamepad text entry
	enum EGamepadTextInputLineMode
	{
		k_EGamepadTextInputLineModeSingleLine = 0,
		k_EGamepadTextInputLineModeMultipleLines = 1
	};

	/** Directions for changing focus between overlays with the gamepad */
	enum EOverlayDirection
	{
		OverlayDirection_Up = 0,
		OverlayDirection_Down = 1,
		OverlayDirection_Left = 2,
		OverlayDirection_Right = 3,
		
		OverlayDirection_Count = 4,
	};

	enum EVROverlayIntersectionMaskPrimitiveType
	{
		OverlayIntersectionPrimitiveType_Rectangle,
		OverlayIntersectionPrimitiveType_Circle,
	};

	struct IntersectionMaskRectangle_t
	{
		float m_flTopLeftX;
		float m_flTopLeftY;
		float m_flWidth;
		float m_flHeight;
	};

	struct IntersectionMaskCircle_t
	{
		float m_flCenterX;
		float m_flCenterY;
		float m_flRadius;
	};

	/** NOTE!!! If you change this you MUST manually update openvr_interop.cs.py and openvr_api_flat.h.py */
	typedef union
	{
		IntersectionMaskRectangle_t m_Rectangle;
		IntersectionMaskCircle_t m_Circle;
	} VROverlayIntersectionMaskPrimitive_Data_t;

	struct VROverlayIntersectionMaskPrimitive_t
	{
		EVROverlayIntersectionMaskPrimitiveType m_nPrimitiveType;
		VROverlayIntersectionMaskPrimitive_Data_t m_Primitive;
	};

	class IVROverlay
	{
	public:

		// ---------------------------------------------
		// Overlay management methods
		// ---------------------------------------------

		/** Finds an existing overlay with the specified key. */
		virtual EVROverlayError FindOverlay( const char *pchOverlayKey, VROverlayHandle_t * pOverlayHandle ) = 0;

		/** Creates a new named overlay. All overlays start hidden and with default settings. */
		virtual EVROverlayError CreateOverlay( const char *pchOverlayKey, const char *pchOverlayName, VROverlayHandle_t * pOverlayHandle ) = 0;

		/** Destroys the specified overlay. When an application calls VR_Shutdown all overlays created by that app are
		* automatically destroyed. */
		virtual EVROverlayError DestroyOverlay( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Specify which overlay to use the high quality render path.  This overlay will be composited in during the distortion pass which
		* results in it drawing on top of everything else, but also at a higher quality as it samples the source texture directly rather than
		* rasterizing into each eye's render texture first.  Because if this, only one of these is supported at any given time.  It is most useful
		* for overlays that are expected to take up most of the user's view (e.g. streaming video).
		* This mode does not support mouse input to your overlay. */
		virtual EVROverlayError SetHighQualityOverlay( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Returns the overlay handle of the current overlay being rendered using the single high quality overlay render path.
		* Otherwise it will return k_ulOverlayHandleInvalid. */
		virtual vr::VROverlayHandle_t GetHighQualityOverlay() = 0;

		/** Fills the provided buffer with the string key of the overlay. Returns the size of buffer required to store the key, including
		* the terminating null character. k_unVROverlayMaxKeyLength will be enough bytes to fit the string. */
		virtual uint32_t GetOverlayKey( VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, EVROverlayError *pError = 0L ) = 0;

		/** Fills the provided buffer with the friendly name of the overlay. Returns the size of buffer required to store the key, including
		* the terminating null character. k_unVROverlayMaxNameLength will be enough bytes to fit the string. */
		virtual uint32_t GetOverlayName( VROverlayHandle_t ulOverlayHandle, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, EVROverlayError *pError = 0L ) = 0;

		/** set the name to use for this overlay */
		virtual EVROverlayError SetOverlayName( VROverlayHandle_t ulOverlayHandle, const char *pchName ) = 0;

		/** Gets the raw image data from an overlay. Overlay image data is always returned as RGBA data, 4 bytes per pixel. If the buffer is not large enough, width and height 
		* will be set and VROverlayError_ArrayTooSmall is returned. */
		virtual EVROverlayError GetOverlayImageData( VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unBufferSize, uint32_t *punWidth, uint32_t *punHeight ) = 0;

		/** returns a string that corresponds with the specified overlay error. The string will be the name 
		* of the error enum value for all valid error codes */
		virtual const char *GetOverlayErrorNameFromEnum( EVROverlayError error ) = 0;

		// ---------------------------------------------
		// Overlay rendering methods
		// ---------------------------------------------

		/** Sets the pid that is allowed to render to this overlay (the creator pid is always allow to render),
		*	by default this is the pid of the process that made the overlay */
		virtual EVROverlayError SetOverlayRenderingPid( VROverlayHandle_t ulOverlayHandle, uint32_t unPID ) = 0;

		/** Gets the pid that is allowed to render to this overlay */
		virtual uint32_t GetOverlayRenderingPid( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Specify flag setting for a given overlay */
		virtual EVROverlayError SetOverlayFlag( VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool bEnabled ) = 0;

		/** Sets flag setting for a given overlay */
		virtual EVROverlayError GetOverlayFlag( VROverlayHandle_t ulOverlayHandle, VROverlayFlags eOverlayFlag, bool *pbEnabled ) = 0;

		/** Sets the color tint of the overlay quad. Use 0.0 to 1.0 per channel. */
		virtual EVROverlayError SetOverlayColor( VROverlayHandle_t ulOverlayHandle, float fRed, float fGreen, float fBlue ) = 0;

		/** Gets the color tint of the overlay quad. */
		virtual EVROverlayError GetOverlayColor( VROverlayHandle_t ulOverlayHandle, float *pfRed, float *pfGreen, float *pfBlue ) = 0;

		/** Sets the alpha of the overlay quad. Use 1.0 for 100 percent opacity to 0.0 for 0 percent opacity. */
		virtual EVROverlayError SetOverlayAlpha( VROverlayHandle_t ulOverlayHandle, float fAlpha ) = 0;

		/** Gets the alpha of the overlay quad. By default overlays are rendering at 100 percent alpha (1.0). */
		virtual EVROverlayError GetOverlayAlpha( VROverlayHandle_t ulOverlayHandle, float *pfAlpha ) = 0;

		/** Sets the aspect ratio of the texels in the overlay. 1.0 means the texels are square. 2.0 means the texels
		* are twice as wide as they are tall. Defaults to 1.0. */
		virtual EVROverlayError SetOverlayTexelAspect( VROverlayHandle_t ulOverlayHandle, float fTexelAspect ) = 0;

		/** Gets the aspect ratio of the texels in the overlay. Defaults to 1.0 */
		virtual EVROverlayError GetOverlayTexelAspect( VROverlayHandle_t ulOverlayHandle, float *pfTexelAspect ) = 0;

		/** Sets the rendering sort order for the overlay. Overlays are rendered this order:
		*      Overlays owned by the scene application
		*      Overlays owned by some other application
		*
		*	Within a category overlays are rendered lowest sort order to highest sort order. Overlays with the same 
		*	sort order are rendered back to front base on distance from the HMD.
		*
		*	Sort order defaults to 0. */
		virtual EVROverlayError SetOverlaySortOrder( VROverlayHandle_t ulOverlayHandle, uint32_t unSortOrder ) = 0;

		/** Gets the sort order of the overlay. See SetOverlaySortOrder for how this works. */
		virtual EVROverlayError GetOverlaySortOrder( VROverlayHandle_t ulOverlayHandle, uint32_t *punSortOrder ) = 0;

		/** Sets the width of the overlay quad in meters. By default overlays are rendered on a quad that is 1 meter across */
		virtual EVROverlayError SetOverlayWidthInMeters( VROverlayHandle_t ulOverlayHandle, float fWidthInMeters ) = 0;

		/** Returns the width of the overlay quad in meters. By default overlays are rendered on a quad that is 1 meter across */
		virtual EVROverlayError GetOverlayWidthInMeters( VROverlayHandle_t ulOverlayHandle, float *pfWidthInMeters ) = 0;

		/** For high-quality curved overlays only, sets the distance range in meters from the overlay used to automatically curve
		* the surface around the viewer.  Min is distance is when the surface will be most curved.  Max is when least curved. */
		virtual EVROverlayError SetOverlayAutoCurveDistanceRangeInMeters( VROverlayHandle_t ulOverlayHandle, float fMinDistanceInMeters, float fMaxDistanceInMeters ) = 0;

		/** For high-quality curved overlays only, gets the distance range in meters from the overlay used to automatically curve
		* the surface around the viewer.  Min is distance is when the surface will be most curved.  Max is when least curved. */
		virtual EVROverlayError GetOverlayAutoCurveDistanceRangeInMeters( VROverlayHandle_t ulOverlayHandle, float *pfMinDistanceInMeters, float *pfMaxDistanceInMeters ) = 0;

		/** Sets the colorspace the overlay texture's data is in.  Defaults to 'auto'.
		* If the texture needs to be resolved, you should call SetOverlayTexture with the appropriate colorspace instead. */
		virtual EVROverlayError SetOverlayTextureColorSpace( VROverlayHandle_t ulOverlayHandle, EColorSpace eTextureColorSpace ) = 0;

		/** Gets the overlay's current colorspace setting. */
		virtual EVROverlayError GetOverlayTextureColorSpace( VROverlayHandle_t ulOverlayHandle, EColorSpace *peTextureColorSpace ) = 0;

		/** Sets the part of the texture to use for the overlay. UV Min is the upper left corner and UV Max is the lower right corner. */
		virtual EVROverlayError SetOverlayTextureBounds( VROverlayHandle_t ulOverlayHandle, const VRTextureBounds_t *pOverlayTextureBounds ) = 0;

		/** Gets the part of the texture to use for the overlay. UV Min is the upper left corner and UV Max is the lower right corner. */
		virtual EVROverlayError GetOverlayTextureBounds( VROverlayHandle_t ulOverlayHandle, VRTextureBounds_t *pOverlayTextureBounds ) = 0;

		/** Gets render model to draw behind this overlay */
		virtual uint32_t GetOverlayRenderModel( vr::VROverlayHandle_t ulOverlayHandle, char *pchValue, uint32_t unBufferSize, HmdColor_t *pColor, vr::EVROverlayError *pError ) = 0;

		/** Sets render model to draw behind this overlay and the vertex color to use, pass null for pColor to match the overlays vertex color. 
			The model is scaled by the same amount as the overlay, with a default of 1m. */
		virtual vr::EVROverlayError SetOverlayRenderModel( vr::VROverlayHandle_t ulOverlayHandle, const char *pchRenderModel, const HmdColor_t *pColor ) = 0;

		/** Returns the transform type of this overlay. */
		virtual EVROverlayError GetOverlayTransformType( VROverlayHandle_t ulOverlayHandle, VROverlayTransformType *peTransformType ) = 0;

		/** Sets the transform to absolute tracking origin. */
		virtual EVROverlayError SetOverlayTransformAbsolute( VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToOverlayTransform ) = 0;

		/** Gets the transform if it is absolute. Returns an error if the transform is some other type. */
		virtual EVROverlayError GetOverlayTransformAbsolute( VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin *peTrackingOrigin, HmdMatrix34_t *pmatTrackingOriginToOverlayTransform ) = 0;

		/** Sets the transform to relative to the transform of the specified tracked device. */
		virtual EVROverlayError SetOverlayTransformTrackedDeviceRelative( VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unTrackedDevice, const HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform ) = 0;

		/** Gets the transform if it is relative to a tracked device. Returns an error if the transform is some other type. */
		virtual EVROverlayError GetOverlayTransformTrackedDeviceRelative( VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punTrackedDevice, HmdMatrix34_t *pmatTrackedDeviceToOverlayTransform ) = 0;

		/** Sets the transform to draw the overlay on a rendermodel component mesh instead of a quad. This will only draw when the system is
		* drawing the device. Overlays with this transform type cannot receive mouse events. */
		virtual EVROverlayError SetOverlayTransformTrackedDeviceComponent( VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unDeviceIndex, const char *pchComponentName ) = 0;

		/** Gets the transform information when the overlay is rendering on a component. */
		virtual EVROverlayError GetOverlayTransformTrackedDeviceComponent( VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t *punDeviceIndex, char *pchComponentName, uint32_t unComponentNameSize ) = 0;

		/** Gets the transform if it is relative to another overlay. Returns an error if the transform is some other type. */
		virtual vr::EVROverlayError GetOverlayTransformOverlayRelative( VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t *ulOverlayHandleParent, HmdMatrix34_t *pmatParentOverlayToOverlayTransform ) = 0;
		
		/** Sets the transform to relative to the transform of the specified overlay. This overlays visibility will also track the parents visibility */
		virtual vr::EVROverlayError SetOverlayTransformOverlayRelative( VROverlayHandle_t ulOverlayHandle, VROverlayHandle_t ulOverlayHandleParent, const HmdMatrix34_t *pmatParentOverlayToOverlayTransform ) = 0;

		/** Shows the VR overlay.  For dashboard overlays, only the Dashboard Manager is allowed to call this. */
		virtual EVROverlayError ShowOverlay( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Hides the VR overlay.  For dashboard overlays, only the Dashboard Manager is allowed to call this. */
		virtual EVROverlayError HideOverlay( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Returns true if the overlay is visible. */
		virtual bool IsOverlayVisible( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Get the transform in 3d space associated with a specific 2d point in the overlay's coordinate space (where 0,0 is the lower left). -Z points out of the overlay */
		virtual EVROverlayError GetTransformForOverlayCoordinates( VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, HmdVector2_t coordinatesInOverlay, HmdMatrix34_t *pmatTransform ) = 0;

		// ---------------------------------------------
		// Overlay input methods
		// ---------------------------------------------

		/** Returns true and fills the event with the next event on the overlay's event queue, if there is one. 
		* If there are no events this method returns false. uncbVREvent should be the size in bytes of the VREvent_t struct */
		virtual bool PollNextOverlayEvent( VROverlayHandle_t ulOverlayHandle, VREvent_t *pEvent, uint32_t uncbVREvent ) = 0;

		/** Returns the current input settings for the specified overlay. */
		virtual EVROverlayError GetOverlayInputMethod( VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod *peInputMethod ) = 0;

		/** Sets the input settings for the specified overlay. */
		virtual EVROverlayError SetOverlayInputMethod( VROverlayHandle_t ulOverlayHandle, VROverlayInputMethod eInputMethod ) = 0;

		/** Gets the mouse scaling factor that is used for mouse events. The actual texture may be a different size, but this is
		* typically the size of the underlying UI in pixels. */
		virtual EVROverlayError GetOverlayMouseScale( VROverlayHandle_t ulOverlayHandle, HmdVector2_t *pvecMouseScale ) = 0;

		/** Sets the mouse scaling factor that is used for mouse events. The actual texture may be a different size, but this is
		* typically the size of the underlying UI in pixels (not in world space). */
		virtual EVROverlayError SetOverlayMouseScale( VROverlayHandle_t ulOverlayHandle, const HmdVector2_t *pvecMouseScale ) = 0;

		/** Computes the overlay-space pixel coordinates of where the ray intersects the overlay with the
		* specified settings. Returns false if there is no intersection. */
		virtual bool ComputeOverlayIntersection( VROverlayHandle_t ulOverlayHandle, const VROverlayIntersectionParams_t *pParams, VROverlayIntersectionResults_t *pResults ) = 0;

		/** Processes mouse input from the specified controller as though it were a mouse pointed at a compositor overlay with the
		* specified settings. The controller is treated like a laser pointer on the -z axis. The point where the laser pointer would
		* intersect with the overlay is the mouse position, the trigger is left mouse, and the track pad is right mouse. 
		*
		* Return true if the controller is pointed at the overlay and an event was generated. */
		virtual bool HandleControllerOverlayInteractionAsMouse( VROverlayHandle_t ulOverlayHandle, TrackedDeviceIndex_t unControllerDeviceIndex ) = 0;

		/** Returns true if the specified overlay is the hover target. An overlay is the hover target when it is the last overlay "moused over" 
		* by the virtual mouse pointer */
		virtual bool IsHoverTargetOverlay( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Returns the current Gamepad focus overlay */
		virtual vr::VROverlayHandle_t GetGamepadFocusOverlay() = 0;

		/** Sets the current Gamepad focus overlay */
		virtual EVROverlayError SetGamepadFocusOverlay( VROverlayHandle_t ulNewFocusOverlay ) = 0;

		/** Sets an overlay's neighbor. This will also set the neighbor of the "to" overlay
		* to point back to the "from" overlay. If an overlay's neighbor is set to invalid both
		* ends will be cleared */
		virtual EVROverlayError SetOverlayNeighbor( EOverlayDirection eDirection, VROverlayHandle_t ulFrom, VROverlayHandle_t ulTo ) = 0;

		/** Changes the Gamepad focus from one overlay to one of its neighbors. Returns VROverlayError_NoNeighbor if there is no
		* neighbor in that direction */
		virtual EVROverlayError MoveGamepadFocusToNeighbor( EOverlayDirection eDirection, VROverlayHandle_t ulFrom ) = 0;

		// ---------------------------------------------
		// Overlay texture methods
		// ---------------------------------------------

		/** Texture to draw for the overlay. This function can only be called by the overlay's creator or renderer process (see SetOverlayRenderingPid) .
		*
		* OpenGL dirty state:
		*	glBindTexture
		*/
		virtual EVROverlayError SetOverlayTexture( VROverlayHandle_t ulOverlayHandle, const Texture_t *pTexture ) = 0;

		/** Use this to tell the overlay system to release the texture set for this overlay. */
		virtual EVROverlayError ClearOverlayTexture( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Separate interface for providing the data as a stream of bytes, but there is an upper bound on data 
		* that can be sent. This function can only be called by the overlay's renderer process. */
		virtual EVROverlayError SetOverlayRaw( VROverlayHandle_t ulOverlayHandle, void *pvBuffer, uint32_t unWidth, uint32_t unHeight, uint32_t unDepth ) = 0;

		/** Separate interface for providing the image through a filename: can be png or jpg, and should not be bigger than 1920x1080.
		* This function can only be called by the overlay's renderer process */
		virtual EVROverlayError SetOverlayFromFile( VROverlayHandle_t ulOverlayHandle, const char *pchFilePath ) = 0;

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
		virtual EVROverlayError GetOverlayTexture( VROverlayHandle_t ulOverlayHandle, void **pNativeTextureHandle, void *pNativeTextureRef, uint32_t *pWidth, uint32_t *pHeight, uint32_t *pNativeFormat, ETextureType *pAPIType, EColorSpace *pColorSpace, VRTextureBounds_t *pTextureBounds ) = 0;

		/** Release the pNativeTextureHandle provided from the GetOverlayTexture call, this allows the system to free the underlying GPU resources for this object,
		* so only do it once you stop rendering this texture.
		*/
		virtual EVROverlayError ReleaseNativeOverlayHandle( VROverlayHandle_t ulOverlayHandle, void *pNativeTextureHandle ) = 0;

		/** Get the size of the overlay texture */
		virtual EVROverlayError GetOverlayTextureSize( VROverlayHandle_t ulOverlayHandle, uint32_t *pWidth, uint32_t *pHeight ) = 0;

		// ----------------------------------------------
		// Dashboard Overlay Methods
		// ----------------------------------------------

		/** Creates a dashboard overlay and returns its handle */
		virtual EVROverlayError CreateDashboardOverlay( const char *pchOverlayKey, const char *pchOverlayFriendlyName, VROverlayHandle_t * pMainHandle, VROverlayHandle_t *pThumbnailHandle ) = 0;

		/** Returns true if the dashboard is visible */
		virtual bool IsDashboardVisible() = 0;

		/** returns true if the dashboard is visible and the specified overlay is the active system Overlay */
		virtual bool IsActiveDashboardOverlay( VROverlayHandle_t ulOverlayHandle ) = 0;

		/** Sets the dashboard overlay to only appear when the specified process ID has scene focus */
		virtual EVROverlayError SetDashboardOverlaySceneProcess( VROverlayHandle_t ulOverlayHandle, uint32_t unProcessId ) = 0;

		/** Gets the process ID that this dashboard overlay requires to have scene focus */
		virtual EVROverlayError GetDashboardOverlaySceneProcess( VROverlayHandle_t ulOverlayHandle, uint32_t *punProcessId ) = 0;

		/** Shows the dashboard. */
		virtual void ShowDashboard( const char *pchOverlayToShow ) = 0;

		/** Returns the tracked device that has the laser pointer in the dashboard */
		virtual vr::TrackedDeviceIndex_t GetPrimaryDashboardDevice() = 0;

		// ---------------------------------------------
		// Keyboard methods
		// ---------------------------------------------
		
		/** Show the virtual keyboard to accept input **/
		virtual EVROverlayError ShowKeyboard( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue ) = 0;

		virtual EVROverlayError ShowKeyboardForOverlay( VROverlayHandle_t ulOverlayHandle, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32_t unCharMax, const char *pchExistingText, bool bUseMinimalMode, uint64_t uUserValue ) = 0;

		/** Get the text that was entered into the text input **/
		virtual uint32_t GetKeyboardText( VR_OUT_STRING() char *pchText, uint32_t cchText ) = 0;

		/** Hide the virtual keyboard **/
		virtual void HideKeyboard() = 0;

		/** Set the position of the keyboard in world space **/
		virtual void SetKeyboardTransformAbsolute( ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t *pmatTrackingOriginToKeyboardTransform ) = 0;

		/** Set the position of the keyboard in overlay space by telling it to avoid a rectangle in the overlay. Rectangle coords have (0,0) in the bottom left **/
		virtual void SetKeyboardPositionForOverlay( VROverlayHandle_t ulOverlayHandle, HmdRect2_t avoidRect ) = 0;

		// ---------------------------------------------
		// Overlay input methods
		// ---------------------------------------------

		/** Sets a list of primitives to be used for controller ray intersection
		* typically the size of the underlying UI in pixels (not in world space). */
		virtual EVROverlayError SetOverlayIntersectionMask( VROverlayHandle_t ulOverlayHandle, VROverlayIntersectionMaskPrimitive_t *pMaskPrimitives, uint32_t unNumMaskPrimitives, uint32_t unPrimitiveSize = sizeof( VROverlayIntersectionMaskPrimitive_t ) ) = 0;

		virtual EVROverlayError GetOverlayFlags( VROverlayHandle_t ulOverlayHandle, uint32_t *pFlags ) = 0;

		// ---------------------------------------------
		// Message box methods
		// ---------------------------------------------

		/** Show the message overlay. This will block and return you a result. **/
		virtual VRMessageOverlayResponse ShowMessageOverlay( const char* pchText, const char* pchCaption, const char* pchButton0Text, const char* pchButton1Text = nullptr, const char* pchButton2Text = nullptr, const char* pchButton3Text = nullptr ) = 0;

		/** If the calling process owns the overlay and it's open, this will close it. **/
		virtual void CloseMessageOverlay() = 0;
	};

	static const char * const IVROverlay_Version = "IVROverlay_016";

} // namespace vr

#include "ivrrendermodels.h"

// ivrextendeddisplay.h
namespace vr
{

	/** NOTE: Use of this interface is not recommended in production applications. It will not work for displays which use
	* direct-to-display mode. Creating our own window is also incompatible with the VR compositor and is not available when the compositor is running. */
	class IVRExtendedDisplay
	{
	public:

		/** Size and position that the window needs to be on the VR display. */
		virtual void GetWindowBounds( int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight ) = 0;

		/** Gets the viewport in the frame buffer to draw the output of the distortion into */
		virtual void GetEyeOutputViewport( EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight ) = 0;

		/** [D3D10/11 Only]
		* Returns the adapter index and output index that the user should pass into EnumAdapters and EnumOutputs
		* to create the device and swap chain in DX10 and DX11. If an error occurs both indices will be set to -1.
		*/
		virtual void GetDXGIOutputInfo( int32_t *pnAdapterIndex, int32_t *pnAdapterOutputIndex ) = 0;

	};

	static const char * const IVRExtendedDisplay_Version = "IVRExtendedDisplay_001";

}


// ivrtrackedcamera.h
namespace vr
{

class IVRTrackedCamera
{
public:
	/** Returns a string for an error */
	virtual const char *GetCameraErrorNameFromEnum( vr::EVRTrackedCameraError eCameraError ) = 0;

	/** For convenience, same as tracked property request Prop_HasCamera_Bool */
	virtual vr::EVRTrackedCameraError HasCamera( vr::TrackedDeviceIndex_t nDeviceIndex, bool *pHasCamera ) = 0;

	/** Gets size of the image frame. */
	virtual vr::EVRTrackedCameraError GetCameraFrameSize( vr::TrackedDeviceIndex_t nDeviceIndex, vr::EVRTrackedCameraFrameType eFrameType, uint32_t *pnWidth, uint32_t *pnHeight, uint32_t *pnFrameBufferSize ) = 0;

	virtual vr::EVRTrackedCameraError GetCameraIntrinsics( vr::TrackedDeviceIndex_t nDeviceIndex, vr::EVRTrackedCameraFrameType eFrameType, vr::HmdVector2_t *pFocalLength, vr::HmdVector2_t *pCenter ) = 0;

	virtual vr::EVRTrackedCameraError GetCameraProjection( vr::TrackedDeviceIndex_t nDeviceIndex, vr::EVRTrackedCameraFrameType eFrameType, float flZNear, float flZFar, vr::HmdMatrix44_t *pProjection ) = 0;

	/** Acquiring streaming service permits video streaming for the caller. Releasing hints the system that video services do not need to be maintained for this client.
	* If the camera has not already been activated, a one time spin up may incur some auto exposure as well as initial streaming frame delays.
	* The camera should be considered a global resource accessible for shared consumption but not exclusive to any caller.
	* The camera may go inactive due to lack of active consumers or headset idleness. */
	virtual vr::EVRTrackedCameraError AcquireVideoStreamingService( vr::TrackedDeviceIndex_t nDeviceIndex, vr::TrackedCameraHandle_t *pHandle ) = 0;
	virtual vr::EVRTrackedCameraError ReleaseVideoStreamingService( vr::TrackedCameraHandle_t hTrackedCamera ) = 0;

	/** Copies the image frame into a caller's provided buffer. The image data is currently provided as RGBA data, 4 bytes per pixel.
	* A caller can provide null for the framebuffer or frameheader if not desired. Requesting the frame header first, followed by the frame buffer allows
	* the caller to determine if the frame as advanced per the frame header sequence. 
	* If there is no frame available yet, due to initial camera spinup or re-activation, the error will be VRTrackedCameraError_NoFrameAvailable.
	* Ideally a caller should be polling at ~16ms intervals */
	virtual vr::EVRTrackedCameraError GetVideoStreamFrameBuffer( vr::TrackedCameraHandle_t hTrackedCamera, vr::EVRTrackedCameraFrameType eFrameType, void *pFrameBuffer, uint32_t nFrameBufferSize, vr::CameraVideoStreamFrameHeader_t *pFrameHeader, uint32_t nFrameHeaderSize ) = 0;

	/** Gets size of the image frame. */
	virtual vr::EVRTrackedCameraError GetVideoStreamTextureSize( vr::TrackedDeviceIndex_t nDeviceIndex, vr::EVRTrackedCameraFrameType eFrameType, vr::VRTextureBounds_t *pTextureBounds, uint32_t *pnWidth, uint32_t *pnHeight ) = 0; 

	/** Access a shared D3D11 texture for the specified tracked camera stream.
	* The camera frame type VRTrackedCameraFrameType_Undistorted is not supported directly as a shared texture. It is an interior subregion of the shared texture VRTrackedCameraFrameType_MaximumUndistorted.
	* Instead, use GetVideoStreamTextureSize() with VRTrackedCameraFrameType_Undistorted to determine the proper interior subregion bounds along with GetVideoStreamTextureD3D11() with
	* VRTrackedCameraFrameType_MaximumUndistorted to provide the texture. The VRTrackedCameraFrameType_MaximumUndistorted will yield an image where the invalid regions are decoded
	* by the alpha channel having a zero component. The valid regions all have a non-zero alpha component. The subregion as described by VRTrackedCameraFrameType_Undistorted 
	* guarantees a rectangle where all pixels are valid. */
	virtual vr::EVRTrackedCameraError GetVideoStreamTextureD3D11( vr::TrackedCameraHandle_t hTrackedCamera, vr::EVRTrackedCameraFrameType eFrameType, void *pD3D11DeviceOrResource, void **ppD3D11ShaderResourceView, vr::CameraVideoStreamFrameHeader_t *pFrameHeader, uint32_t nFrameHeaderSize ) = 0;

	/** Access a shared GL texture for the specified tracked camera stream */
	virtual vr::EVRTrackedCameraError GetVideoStreamTextureGL( vr::TrackedCameraHandle_t hTrackedCamera, vr::EVRTrackedCameraFrameType eFrameType, vr::glUInt_t *pglTextureId, vr::CameraVideoStreamFrameHeader_t *pFrameHeader, uint32_t nFrameHeaderSize ) = 0;
	virtual vr::EVRTrackedCameraError ReleaseVideoStreamTextureGL( vr::TrackedCameraHandle_t hTrackedCamera, vr::glUInt_t glTextureId ) = 0;
};

static const char * const IVRTrackedCamera_Version = "IVRTrackedCamera_003";

} // namespace vr


// ivrscreenshots.h
namespace vr
{

/** Errors that can occur with the VR compositor */
enum EVRScreenshotError
{
	VRScreenshotError_None							= 0,
	VRScreenshotError_RequestFailed					= 1,
	VRScreenshotError_IncompatibleVersion			= 100,
	VRScreenshotError_NotFound						= 101,
	VRScreenshotError_BufferTooSmall				= 102,
	VRScreenshotError_ScreenshotAlreadyInProgress	= 108,
};

/** Allows the application to generate screenshots */
class IVRScreenshots
{
public:
	/** Request a screenshot of the requested type.
	 *  A request of the VRScreenshotType_Stereo type will always
	 *  work. Other types will depend on the underlying application
	 *  support.
	 *  The first file name is for the preview image and should be a
	 *  regular screenshot (ideally from the left eye). The second
	 *  is the VR screenshot in the correct format. They should be
	 *  in the same aspect ratio.  Formats per type:
	 *  VRScreenshotType_Mono: the VR filename is ignored (can be
	 *  nullptr), this is a normal flat single shot.
	 *  VRScreenshotType_Stereo:  The VR image should be a
	 *  side-by-side with the left eye image on the left.
	 *  VRScreenshotType_Cubemap: The VR image should be six square
	 *  images composited horizontally.
	 *  VRScreenshotType_StereoPanorama: above/below with left eye
	 *  panorama being the above image.  Image is typically square
	 *  with the panorama being 2x horizontal.
	 *  
	 *  Note that the VR dashboard will call this function when
	 *  the user presses the screenshot binding (currently System
	 *  Button + Trigger).  If Steam is running, the destination
	 *  file names will be in %TEMP% and will be copied into
	 *  Steam's screenshot library for the running application
	 *  once SubmitScreenshot() is called.
	 *  If Steam is not running, the paths will be in the user's
	 *  documents folder under Documents\SteamVR\Screenshots.
	 *  Other VR applications can call this to initiate a
	 *  screenshot outside of user control.
	 *  The destination file names do not need an extension,
	 *  will be replaced with the correct one for the format
	 *  which is currently .png. */
	virtual vr::EVRScreenshotError RequestScreenshot( vr::ScreenshotHandle_t *pOutScreenshotHandle, vr::EVRScreenshotType type, const char *pchPreviewFilename, const char *pchVRFilename ) = 0;

	/** Called by the running VR application to indicate that it
	 *  wishes to be in charge of screenshots.  If the
	 *  application does not call this, the Compositor will only
	 *  support VRScreenshotType_Stereo screenshots that will be
	 *  captured without notification to the running app.
	 *  Once hooked your application will receive a
	 *  VREvent_RequestScreenshot event when the user presses the
	 *  buttons to take a screenshot. */
	virtual vr::EVRScreenshotError HookScreenshot( VR_ARRAY_COUNT( numTypes ) const vr::EVRScreenshotType *pSupportedTypes, int numTypes ) = 0;

	/** When your application receives a
	 *  VREvent_RequestScreenshot event, call these functions to get
	 *  the details of the screenshot request. */
	virtual vr::EVRScreenshotType GetScreenshotPropertyType( vr::ScreenshotHandle_t screenshotHandle, vr::EVRScreenshotError *pError ) = 0;

	/** Get the filename for the preview or vr image (see
	 *  vr::EScreenshotPropertyFilenames).  The return value is
	 *  the size of the string.   */
 	virtual uint32_t GetScreenshotPropertyFilename( vr::ScreenshotHandle_t screenshotHandle, vr::EVRScreenshotPropertyFilenames filenameType, VR_OUT_STRING() char *pchFilename, uint32_t cchFilename, vr::EVRScreenshotError *pError ) = 0;

	/** Call this if the application is taking the screen shot
	 *  will take more than a few ms processing. This will result
	 *  in an overlay being presented that shows a completion
	 *  bar. */
	virtual vr::EVRScreenshotError UpdateScreenshotProgress( vr::ScreenshotHandle_t screenshotHandle, float flProgress ) = 0;

	/** Tells the compositor to take an internal screenshot of
	 *  type VRScreenshotType_Stereo. It will take the current
	 *  submitted scene textures of the running application and
	 *  write them into the preview image and a side-by-side file
	 *  for the VR image.
	 *  This is similar to request screenshot, but doesn't ever
	 *  talk to the application, just takes the shot and submits. */
	virtual vr::EVRScreenshotError TakeStereoScreenshot( vr::ScreenshotHandle_t *pOutScreenshotHandle, const char *pchPreviewFilename, const char *pchVRFilename ) = 0;

	/** Submit the completed screenshot.  If Steam is running
	 *  this will call into the Steam client and upload the
	 *  screenshot to the screenshots section of the library for
	 *  the running application.  If Steam is not running, this
	 *  function will display a notification to the user that the
	 *  screenshot was taken. The paths should be full paths with
	 *  extensions.
	 *  File paths should be absolute including extensions.
	 *  screenshotHandle can be k_unScreenshotHandleInvalid if this
	 *  was a new shot taking by the app to be saved and not
	 *  initiated by a user (achievement earned or something) */
	virtual vr::EVRScreenshotError SubmitScreenshot( vr::ScreenshotHandle_t screenshotHandle, vr::EVRScreenshotType type, const char *pchSourcePreviewFilename, const char *pchSourceVRFilename ) = 0;
};

static const char * const IVRScreenshots_Version = "IVRScreenshots_001";

} // namespace vr



// ivrresources.h
namespace vr
{

class IVRResources
{
public:

	// ------------------------------------
	// Shared Resource Methods
	// ------------------------------------

	/** Loads the specified resource into the provided buffer if large enough.
	* Returns the size in bytes of the buffer required to hold the specified resource. */
	virtual uint32_t LoadSharedResource( const char *pchResourceName, char *pchBuffer, uint32_t unBufferLen ) = 0;

	/** Provides the full path to the specified resource. Resource names can include named directories for
	* drivers and other things, and this resolves all of those and returns the actual physical path. 
	* pchResourceTypeDirectory is the subdirectory of resources to look in. */
	virtual uint32_t GetResourceFullPath( const char *pchResourceName, const char *pchResourceTypeDirectory, char *pchPathBuffer, uint32_t unBufferLen ) = 0;
};

static const char * const IVRResources_Version = "IVRResources_001";


}
// ivrdrivermanager.h
namespace vr
{

class IVRDriverManager
{
public:
	virtual uint32_t GetDriverCount() const = 0;

	/** Returns the length of the number of bytes necessary to hold this string including the trailing null. */
	virtual uint32_t GetDriverName( vr::DriverId_t nDriver, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize ) = 0;
};

static const char * const IVRDriverManager_Version = "IVRDriverManager_001";

} // namespace vr


// End

#endif // _OPENVR_API

