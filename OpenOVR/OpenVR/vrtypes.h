
// vrtypes.h
#ifndef _INCLUDE_VRTYPES_H
#define _INCLUDE_VRTYPES_H

#include <stdint.h>

// Forward declarations to avoid requiring vulkan.h
struct VkDevice_T;
struct VkPhysicalDevice_T;
struct VkInstance_T;
struct VkQueue_T;

// Forward declarations to avoid requiring d3d12.h
struct ID3D12Resource;
struct ID3D12CommandQueue;

namespace vr {
#pragma pack( push, 8 )

	typedef void* glSharedTextureHandle_t;
	typedef int32_t glInt_t;
	typedef uint32_t glUInt_t;

	// right-handed system
	// +y is up
	// +x is to the right
	// -z is going away from you
	// Distance unit is  meters
	struct HmdMatrix34_t {
		float m[3][4];
	};

	struct HmdMatrix44_t {
		float m[4][4];
	};

	struct HmdVector3_t {
		float v[3];
	};

	struct HmdVector4_t {
		float v[4];
	};

	struct HmdVector3d_t {
		double v[3];
	};

	struct HmdVector2_t {
		float v[2];
	};

	struct HmdQuaternion_t {
		double w, x, y, z;
	};

	struct HmdColor_t {
		float r, g, b, a;
	};

	struct HmdQuad_t {
		HmdVector3_t vCorners[4];
	};

	struct HmdRect2_t {
		HmdVector2_t vTopLeft;
		HmdVector2_t vBottomRight;
	};

	/** Used to return the post-distortion UVs for each color channel.
	* UVs range from 0 to 1 with 0,0 in the upper left corner of the
	* source render target. The 0,0 to 1,1 range covers a single eye. */
	struct DistortionCoordinates_t {
		float rfRed[2];
		float rfGreen[2];
		float rfBlue[2];
	};

	enum EVREye {
		Eye_Left = 0,
		Eye_Right = 1
	};

	enum ETextureType {
		TextureType_DirectX = 0, // Handle is an ID3D11Texture
		TextureType_OpenGL = 1,  // Handle is an OpenGL texture name or an OpenGL render buffer name, depending on submit flags
		TextureType_Vulkan = 2, // Handle is a pointer to a VRVulkanTextureData_t structure
		TextureType_IOSurface = 3, // Handle is a macOS cross-process-sharable IOSurfaceRef
		TextureType_DirectX12 = 4, // Handle is a pointer to a D3D12TextureData_t structure
	};

	enum EColorSpace {
		ColorSpace_Auto = 0,	// Assumes 'gamma' for 8-bit per component formats, otherwise 'linear'.  This mirrors the DXGI formats which have _SRGB variants.
		ColorSpace_Gamma = 1,	// Texture data can be displayed directly on the display without any conversion (a.k.a. display native format).
		ColorSpace_Linear = 2,	// Same as gamma but has been converted to a linear representation using DXGI's sRGB conversion algorithm.
	};

	struct Texture_t {
		void* handle; // See ETextureType definition above
		ETextureType eType;
		EColorSpace eColorSpace;
	};

	// Handle to a shared texture (HANDLE on Windows obtained using OpenSharedResource).
	typedef uint64_t SharedTextureHandle_t;
#define INVALID_SHARED_TEXTURE_HANDLE	((vr::SharedTextureHandle_t)0)

	enum ETrackingResult {
		TrackingResult_Uninitialized = 1,

		TrackingResult_Calibrating_InProgress = 100,
		TrackingResult_Calibrating_OutOfRange = 101,

		TrackingResult_Running_OK = 200,
		TrackingResult_Running_OutOfRange = 201,
	};

	typedef uint32_t DriverId_t;
	static const uint32_t k_nDriverNone = 0xFFFFFFFF;

	static const uint32_t k_unMaxDriverDebugResponseSize = 32768;

	/** Used to pass device IDs to API calls */
	typedef uint32_t TrackedDeviceIndex_t;
	static const uint32_t k_unTrackedDeviceIndex_Hmd = 0;
	static const uint32_t k_unMaxTrackedDeviceCount = 16;
	static const uint32_t k_unTrackedDeviceIndexOther = 0xFFFFFFFE;
	static const uint32_t k_unTrackedDeviceIndexInvalid = 0xFFFFFFFF;

	/** Describes what kind of object is being tracked at a given ID */
	enum ETrackedDeviceClass {
		TrackedDeviceClass_Invalid = 0,				// the ID was not valid.
		TrackedDeviceClass_HMD = 1,					// Head-Mounted Displays
		TrackedDeviceClass_Controller = 2,			// Tracked controllers
		TrackedDeviceClass_GenericTracker = 3,		// Generic trackers, similar to controllers
		TrackedDeviceClass_TrackingReference = 4,	// Camera and base stations that serve as tracking reference points
		TrackedDeviceClass_DisplayRedirect = 5,		// Accessories that aren't necessarily tracked themselves, but may redirect video output from other tracked devices
	};


	/** Describes what specific role associated with a tracked device */
	enum ETrackedControllerRole {
		TrackedControllerRole_Invalid = 0,					// Invalid value for controller type
		TrackedControllerRole_LeftHand = 1,					// Tracked device associated with the left hand
		TrackedControllerRole_RightHand = 2,				// Tracked device associated with the right hand
	};


	/** describes a single pose for a tracked object */
	struct TrackedDevicePose_t {
		HmdMatrix34_t mDeviceToAbsoluteTracking;
		HmdVector3_t vVelocity;				// velocity in tracker space in m/s
		HmdVector3_t vAngularVelocity;		// angular velocity in radians/s (?)
		ETrackingResult eTrackingResult;
		bool bPoseIsValid;

		// This indicates that there is a device connected for this spot in the pose array.
		// It could go from true to false if the user unplugs the device.
		bool bDeviceIsConnected;
	};

	/** Identifies which style of tracking origin the application wants to use
	* for the poses it is requesting */
	enum ETrackingUniverseOrigin {
		TrackingUniverseSeated = 0,		// Poses are provided relative to the seated zero pose
		TrackingUniverseStanding = 1,	// Poses are provided relative to the safe bounds configured by the user
		TrackingUniverseRawAndUncalibrated = 2,	// Poses are provided in the coordinate system defined by the driver.  It has Y up and is unified for devices of the same driver. You usually don't want this one.
	};

	// Refers to a single container of properties
	typedef uint64_t PropertyContainerHandle_t;
	typedef uint32_t PropertyTypeTag_t;

	static const PropertyContainerHandle_t k_ulInvalidPropertyContainer = 0;
	static const PropertyTypeTag_t k_unInvalidPropertyTag = 0;

	// Use these tags to set/get common types as struct properties
	static const PropertyTypeTag_t k_unFloatPropertyTag = 1;
	static const PropertyTypeTag_t k_unInt32PropertyTag = 2;
	static const PropertyTypeTag_t k_unUint64PropertyTag = 3;
	static const PropertyTypeTag_t k_unBoolPropertyTag = 4;
	static const PropertyTypeTag_t k_unStringPropertyTag = 5;

	static const PropertyTypeTag_t k_unHmdMatrix34PropertyTag = 20;
	static const PropertyTypeTag_t k_unHmdMatrix44PropertyTag = 21;
	static const PropertyTypeTag_t k_unHmdVector3PropertyTag = 22;
	static const PropertyTypeTag_t k_unHmdVector4PropertyTag = 23;

	static const PropertyTypeTag_t k_unHiddenAreaPropertyTag = 30;

	static const PropertyTypeTag_t k_unOpenVRInternalReserved_Start = 1000;
	static const PropertyTypeTag_t k_unOpenVRInternalReserved_End = 10000;


	/** Each entry in this enum represents a property that can be retrieved about a
	* tracked device. Many fields are only valid for one ETrackedDeviceClass. */
	enum ETrackedDeviceProperty {
		Prop_Invalid = 0,

		// general properties that apply to all device classes
		Prop_TrackingSystemName_String = 1000,
		Prop_ModelNumber_String = 1001,
		Prop_SerialNumber_String = 1002,
		Prop_RenderModelName_String = 1003,
		Prop_WillDriftInYaw_Bool = 1004,
		Prop_ManufacturerName_String = 1005,
		Prop_TrackingFirmwareVersion_String = 1006,
		Prop_HardwareRevision_String = 1007,
		Prop_AllWirelessDongleDescriptions_String = 1008,
		Prop_ConnectedWirelessDongle_String = 1009,
		Prop_DeviceIsWireless_Bool = 1010,
		Prop_DeviceIsCharging_Bool = 1011,
		Prop_DeviceBatteryPercentage_Float = 1012, // 0 is empty, 1 is full
		Prop_StatusDisplayTransform_Matrix34 = 1013,
		Prop_Firmware_UpdateAvailable_Bool = 1014,
		Prop_Firmware_ManualUpdate_Bool = 1015,
		Prop_Firmware_ManualUpdateURL_String = 1016,
		Prop_HardwareRevision_Uint64 = 1017,
		Prop_FirmwareVersion_Uint64 = 1018,
		Prop_FPGAVersion_Uint64 = 1019,
		Prop_VRCVersion_Uint64 = 1020,
		Prop_RadioVersion_Uint64 = 1021,
		Prop_DongleVersion_Uint64 = 1022,
		Prop_BlockServerShutdown_Bool = 1023,
		Prop_CanUnifyCoordinateSystemWithHmd_Bool = 1024,
		Prop_ContainsProximitySensor_Bool = 1025,
		Prop_DeviceProvidesBatteryStatus_Bool = 1026,
		Prop_DeviceCanPowerOff_Bool = 1027,
		Prop_Firmware_ProgrammingTarget_String = 1028,
		Prop_DeviceClass_Int32 = 1029,
		Prop_HasCamera_Bool = 1030,
		Prop_DriverVersion_String = 1031,
		Prop_Firmware_ForceUpdateRequired_Bool = 1032,
		Prop_ViveSystemButtonFixRequired_Bool = 1033,
		Prop_ParentDriver_Uint64 = 1034,
		Prop_ResourceRoot_String = 1035,

		// Properties that are unique to TrackedDeviceClass_HMD
		Prop_ReportsTimeSinceVSync_Bool = 2000,
		Prop_SecondsFromVsyncToPhotons_Float = 2001,
		Prop_DisplayFrequency_Float = 2002,
		Prop_UserIpdMeters_Float = 2003,
		Prop_CurrentUniverseId_Uint64 = 2004,
		Prop_PreviousUniverseId_Uint64 = 2005,
		Prop_DisplayFirmwareVersion_Uint64 = 2006,
		Prop_IsOnDesktop_Bool = 2007,
		Prop_DisplayMCType_Int32 = 2008,
		Prop_DisplayMCOffset_Float = 2009,
		Prop_DisplayMCScale_Float = 2010,
		Prop_EdidVendorID_Int32 = 2011,
		Prop_DisplayMCImageLeft_String = 2012,
		Prop_DisplayMCImageRight_String = 2013,
		Prop_DisplayGCBlackClamp_Float = 2014,
		Prop_EdidProductID_Int32 = 2015,
		Prop_CameraToHeadTransform_Matrix34 = 2016,
		Prop_DisplayGCType_Int32 = 2017,
		Prop_DisplayGCOffset_Float = 2018,
		Prop_DisplayGCScale_Float = 2019,
		Prop_DisplayGCPrescale_Float = 2020,
		Prop_DisplayGCImage_String = 2021,
		Prop_LensCenterLeftU_Float = 2022,
		Prop_LensCenterLeftV_Float = 2023,
		Prop_LensCenterRightU_Float = 2024,
		Prop_LensCenterRightV_Float = 2025,
		Prop_UserHeadToEyeDepthMeters_Float = 2026,
		Prop_CameraFirmwareVersion_Uint64 = 2027,
		Prop_CameraFirmwareDescription_String = 2028,
		Prop_DisplayFPGAVersion_Uint64 = 2029,
		Prop_DisplayBootloaderVersion_Uint64 = 2030,
		Prop_DisplayHardwareVersion_Uint64 = 2031,
		Prop_AudioFirmwareVersion_Uint64 = 2032,
		Prop_CameraCompatibilityMode_Int32 = 2033,
		Prop_ScreenshotHorizontalFieldOfViewDegrees_Float = 2034,
		Prop_ScreenshotVerticalFieldOfViewDegrees_Float = 2035,
		Prop_DisplaySuppressed_Bool = 2036,
		Prop_DisplayAllowNightMode_Bool = 2037,
		Prop_DisplayMCImageWidth_Int32 = 2038,
		Prop_DisplayMCImageHeight_Int32 = 2039,
		Prop_DisplayMCImageNumChannels_Int32 = 2040,
		Prop_DisplayMCImageData_Binary = 2041,
		Prop_SecondsFromPhotonsToVblank_Float = 2042,
		Prop_DriverDirectModeSendsVsyncEvents_Bool = 2043,
		Prop_DisplayDebugMode_Bool = 2044,
		Prop_GraphicsAdapterLuid_Uint64 = 2045,
		Prop_DriverProvidedChaperonePath_String = 2048,

		// Properties that are unique to TrackedDeviceClass_Controller
		Prop_AttachedDeviceId_String = 3000,
		Prop_SupportedButtons_Uint64 = 3001,
		Prop_Axis0Type_Int32 = 3002, // Return value is of type EVRControllerAxisType
		Prop_Axis1Type_Int32 = 3003, // Return value is of type EVRControllerAxisType
		Prop_Axis2Type_Int32 = 3004, // Return value is of type EVRControllerAxisType
		Prop_Axis3Type_Int32 = 3005, // Return value is of type EVRControllerAxisType
		Prop_Axis4Type_Int32 = 3006, // Return value is of type EVRControllerAxisType
		Prop_ControllerRoleHint_Int32 = 3007, // Return value is of type ETrackedControllerRole

											  // Properties that are unique to TrackedDeviceClass_TrackingReference
											  Prop_FieldOfViewLeftDegrees_Float = 4000,
											  Prop_FieldOfViewRightDegrees_Float = 4001,
											  Prop_FieldOfViewTopDegrees_Float = 4002,
											  Prop_FieldOfViewBottomDegrees_Float = 4003,
											  Prop_TrackingRangeMinimumMeters_Float = 4004,
											  Prop_TrackingRangeMaximumMeters_Float = 4005,
											  Prop_ModeLabel_String = 4006,

											  // Properties that are used for user interface like icons names
											  Prop_IconPathName_String = 5000, // DEPRECATED. Value not referenced. Now expected to be part of icon path properties.
											  Prop_NamedIconPathDeviceOff_String = 5001, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceSearching_String = 5002, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceSearchingAlert_String = 5003, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceReady_String = 5004, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceReadyAlert_String = 5005, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceNotReady_String = 5006, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceStandby_String = 5007, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others
											  Prop_NamedIconPathDeviceAlertLow_String = 5008, // {driver}/icons/icon_filename - PNG for static icon, or GIF for animation, 50x32 for headsets and 32x32 for others

																							  // Properties that are used by helpers, but are opaque to applications
																							  Prop_DisplayHiddenArea_Binary_Start = 5100,
																							  Prop_DisplayHiddenArea_Binary_End = 5150,

																							  // Properties that are unique to drivers
																							  Prop_UserConfigPath_String = 6000,
																							  Prop_InstallPath_String = 6001,
																							  Prop_HasDisplayComponent_Bool = 6002,
																							  Prop_HasControllerComponent_Bool = 6003,
																							  Prop_HasCameraComponent_Bool = 6004,
																							  Prop_HasDriverDirectModeComponent_Bool = 6005,
																							  Prop_HasVirtualDisplayComponent_Bool = 6006,

																							  // Vendors are free to expose private debug data in this reserved region
																							  Prop_VendorSpecific_Reserved_Start = 10000,
																							  Prop_VendorSpecific_Reserved_End = 10999,
	};

	/** No string property will ever be longer than this length */
	static const uint32_t k_unMaxPropertyStringSize = 32 * 1024;

	/** Used to return errors that occur when reading properties. */
	enum ETrackedPropertyError {
		TrackedProp_Success = 0,
		TrackedProp_WrongDataType = 1,
		TrackedProp_WrongDeviceClass = 2,
		TrackedProp_BufferTooSmall = 3,
		TrackedProp_UnknownProperty = 4, // Driver has not set the property (and may not ever).
		TrackedProp_InvalidDevice = 5,
		TrackedProp_CouldNotContactServer = 6,
		TrackedProp_ValueNotProvidedByDevice = 7,
		TrackedProp_StringExceedsMaximumLength = 8,
		TrackedProp_NotYetAvailable = 9, // The property value isn't known yet, but is expected soon. Call again later.
		TrackedProp_PermissionDenied = 10,
		TrackedProp_InvalidOperation = 11,
	};

	/** Allows the application to control what part of the provided texture will be used in the
	* frame buffer. */
	struct VRTextureBounds_t {
		float uMin, vMin;
		float uMax, vMax;
	};

	/** Allows specifying pose used to render provided scene texture (if different from value returned by WaitGetPoses). */
	struct VRTextureWithPose_t : public Texture_t {
		HmdMatrix34_t mDeviceToAbsoluteTracking; // Actual pose used to render scene textures.
	};

	/** Allows the application to control how scene textures are used by the compositor when calling Submit. */
	enum EVRSubmitFlags {
		// Simple render path. App submits rendered left and right eye images with no lens distortion correction applied.
		Submit_Default = 0x00,

		// App submits final left and right eye images with lens distortion already applied (lens distortion makes the images appear
		// barrel distorted with chromatic aberration correction applied). The app would have used the data returned by
		// vr::IVRSystem::ComputeDistortion() to apply the correct distortion to the rendered images before calling Submit().
		Submit_LensDistortionAlreadyApplied = 0x01,

		// If the texture pointer passed in is actually a renderbuffer (e.g. for MSAA in OpenGL) then set this flag.
		Submit_GlRenderBuffer = 0x02,

		// Do not use
		Submit_Reserved = 0x04,

		// Set to indicate that pTexture is a pointer to a VRTextureWithPose_t.
		Submit_TextureWithPose = 0x08,
	};

	/** Data required for passing Vulkan textures to IVRCompositor::Submit.
	* Be sure to call OpenVR_Shutdown before destroying these resources. */
	struct VRVulkanTextureData_t {
		uint64_t m_nImage; // VkImage
		VkDevice_T *m_pDevice;
		VkPhysicalDevice_T *m_pPhysicalDevice;
		VkInstance_T *m_pInstance;
		VkQueue_T *m_pQueue;
		uint32_t m_nQueueFamilyIndex;
		uint32_t m_nWidth, m_nHeight, m_nFormat, m_nSampleCount;
	};

	/** Data required for passing D3D12 textures to IVRCompositor::Submit.
	* Be sure to call OpenVR_Shutdown before destroying these resources. */
	struct D3D12TextureData_t {
		ID3D12Resource *m_pResource;
		ID3D12CommandQueue *m_pCommandQueue;
		uint32_t m_nNodeMask;
	};

	/** Status of the overall system or tracked objects */
	enum EVRState {
		VRState_Undefined = -1,
		VRState_Off = 0,
		VRState_Searching = 1,
		VRState_Searching_Alert = 2,
		VRState_Ready = 3,
		VRState_Ready_Alert = 4,
		VRState_NotReady = 5,
		VRState_Standby = 6,
		VRState_Ready_Alert_Low = 7,
	};

	/** The types of events that could be posted (and what the parameters mean for each event type) */
	enum EVREventType {
		VREvent_None = 0,

		VREvent_TrackedDeviceActivated = 100,
		VREvent_TrackedDeviceDeactivated = 101,
		VREvent_TrackedDeviceUpdated = 102,
		VREvent_TrackedDeviceUserInteractionStarted = 103,
		VREvent_TrackedDeviceUserInteractionEnded = 104,
		VREvent_IpdChanged = 105,
		VREvent_EnterStandbyMode = 106,
		VREvent_LeaveStandbyMode = 107,
		VREvent_TrackedDeviceRoleChanged = 108,
		VREvent_WatchdogWakeUpRequested = 109,
		VREvent_LensDistortionChanged = 110,
		VREvent_PropertyChanged = 111,
		VREvent_WirelessDisconnect = 112,
		VREvent_WirelessReconnect = 113,

		VREvent_ButtonPress = 200, // data is controller
		VREvent_ButtonUnpress = 201, // data is controller
		VREvent_ButtonTouch = 202, // data is controller
		VREvent_ButtonUntouch = 203, // data is controller

		VREvent_MouseMove = 300, // data is mouse
		VREvent_MouseButtonDown = 301, // data is mouse
		VREvent_MouseButtonUp = 302, // data is mouse
		VREvent_FocusEnter = 303, // data is overlay
		VREvent_FocusLeave = 304, // data is overlay
		VREvent_Scroll = 305, // data is mouse
		VREvent_TouchPadMove = 306, // data is mouse
		VREvent_OverlayFocusChanged = 307, // data is overlay, global event

		VREvent_InputFocusCaptured = 400, // data is process DEPRECATED
		VREvent_InputFocusReleased = 401, // data is process DEPRECATED
		VREvent_SceneFocusLost = 402, // data is process
		VREvent_SceneFocusGained = 403, // data is process
		VREvent_SceneApplicationChanged = 404, // data is process - The App actually drawing the scene changed (usually to or from the compositor)
		VREvent_SceneFocusChanged = 405, // data is process - New app got access to draw the scene
		VREvent_InputFocusChanged = 406, // data is process
		VREvent_SceneApplicationSecondaryRenderingStarted = 407, // data is process

		VREvent_HideRenderModels = 410, // Sent to the scene application to request hiding render models temporarily
		VREvent_ShowRenderModels = 411, // Sent to the scene application to request restoring render model visibility

		VREvent_OverlayShown = 500,
		VREvent_OverlayHidden = 501,
		VREvent_DashboardActivated = 502,
		VREvent_DashboardDeactivated = 503,
		VREvent_DashboardThumbSelected = 504, // Sent to the overlay manager - data is overlay
		VREvent_DashboardRequested = 505, // Sent to the overlay manager - data is overlay
		VREvent_ResetDashboard = 506, // Send to the overlay manager
		VREvent_RenderToast = 507, // Send to the dashboard to render a toast - data is the notification ID
		VREvent_ImageLoaded = 508, // Sent to overlays when a SetOverlayRaw or SetOverlayFromFile call finishes loading
		VREvent_ShowKeyboard = 509, // Sent to keyboard renderer in the dashboard to invoke it
		VREvent_HideKeyboard = 510, // Sent to keyboard renderer in the dashboard to hide it
		VREvent_OverlayGamepadFocusGained = 511, // Sent to an overlay when IVROverlay::SetFocusOverlay is called on it
		VREvent_OverlayGamepadFocusLost = 512, // Send to an overlay when it previously had focus and IVROverlay::SetFocusOverlay is called on something else
		VREvent_OverlaySharedTextureChanged = 513,
		VREvent_DashboardGuideButtonDown = 514,
		VREvent_DashboardGuideButtonUp = 515,
		VREvent_ScreenshotTriggered = 516, // Screenshot button combo was pressed, Dashboard should request a screenshot
		VREvent_ImageFailed = 517, // Sent to overlays when a SetOverlayRaw or SetOverlayfromFail fails to load
		VREvent_DashboardOverlayCreated = 518,

		// Screenshot API
		VREvent_RequestScreenshot = 520, // Sent by vrclient application to compositor to take a screenshot
		VREvent_ScreenshotTaken = 521, // Sent by compositor to the application that the screenshot has been taken
		VREvent_ScreenshotFailed = 522, // Sent by compositor to the application that the screenshot failed to be taken
		VREvent_SubmitScreenshotToDashboard = 523, // Sent by compositor to the dashboard that a completed screenshot was submitted
		VREvent_ScreenshotProgressToDashboard = 524, // Sent by compositor to the dashboard that a completed screenshot was submitted

		VREvent_PrimaryDashboardDeviceChanged = 525,

		VREvent_Notification_Shown = 600,
		VREvent_Notification_Hidden = 601,
		VREvent_Notification_BeginInteraction = 602,
		VREvent_Notification_Destroyed = 603,

		VREvent_Quit = 700, // data is process
		VREvent_ProcessQuit = 701, // data is process
		VREvent_QuitAborted_UserPrompt = 702, // data is process
		VREvent_QuitAcknowledged = 703, // data is process
		VREvent_DriverRequestedQuit = 704, // The driver has requested that SteamVR shut down

		VREvent_ChaperoneDataHasChanged = 800,
		VREvent_ChaperoneUniverseHasChanged = 801,
		VREvent_ChaperoneTempDataHasChanged = 802,
		VREvent_ChaperoneSettingsHaveChanged = 803,
		VREvent_SeatedZeroPoseReset = 804,

		VREvent_AudioSettingsHaveChanged = 820,

		VREvent_BackgroundSettingHasChanged = 850,
		VREvent_CameraSettingsHaveChanged = 851,
		VREvent_ReprojectionSettingHasChanged = 852,
		VREvent_ModelSkinSettingsHaveChanged = 853,
		VREvent_EnvironmentSettingsHaveChanged = 854,
		VREvent_PowerSettingsHaveChanged = 855,
		VREvent_EnableHomeAppSettingsHaveChanged = 856,

		VREvent_StatusUpdate = 900,

		VREvent_MCImageUpdated = 1000,

		VREvent_FirmwareUpdateStarted = 1100,
		VREvent_FirmwareUpdateFinished = 1101,

		VREvent_KeyboardClosed = 1200,
		VREvent_KeyboardCharInput = 1201,
		VREvent_KeyboardDone = 1202, // Sent when DONE button clicked on keyboard

		VREvent_ApplicationTransitionStarted = 1300,
		VREvent_ApplicationTransitionAborted = 1301,
		VREvent_ApplicationTransitionNewAppStarted = 1302,
		VREvent_ApplicationListUpdated = 1303,
		VREvent_ApplicationMimeTypeLoad = 1304,
		VREvent_ApplicationTransitionNewAppLaunchComplete = 1305,
		VREvent_ProcessConnected = 1306,
		VREvent_ProcessDisconnected = 1307,

		VREvent_Compositor_MirrorWindowShown = 1400,
		VREvent_Compositor_MirrorWindowHidden = 1401,
		VREvent_Compositor_ChaperoneBoundsShown = 1410,
		VREvent_Compositor_ChaperoneBoundsHidden = 1411,

		VREvent_TrackedCamera_StartVideoStream = 1500,
		VREvent_TrackedCamera_StopVideoStream = 1501,
		VREvent_TrackedCamera_PauseVideoStream = 1502,
		VREvent_TrackedCamera_ResumeVideoStream = 1503,
		VREvent_TrackedCamera_EditingSurface = 1550,

		VREvent_PerformanceTest_EnableCapture = 1600,
		VREvent_PerformanceTest_DisableCapture = 1601,
		VREvent_PerformanceTest_FidelityLevel = 1602,

		VREvent_MessageOverlay_Closed = 1650,
		VREvent_MessageOverlayCloseRequested = 1651,

		// Vendors are free to expose private events in this reserved region
		VREvent_VendorSpecific_Reserved_Start = 10000,
		VREvent_VendorSpecific_Reserved_End = 19999,
	};


	/** Level of Hmd activity */
	// UserInteraction_Timeout means the device is in the process of timing out.
	// InUse = ( k_EDeviceActivityLevel_UserInteraction || k_EDeviceActivityLevel_UserInteraction_Timeout )
	// VREvent_TrackedDeviceUserInteractionStarted fires when the devices transitions from Standby -> UserInteraction or Idle -> UserInteraction.
	// VREvent_TrackedDeviceUserInteractionEnded fires when the devices transitions from UserInteraction_Timeout -> Idle
	enum EDeviceActivityLevel {
		k_EDeviceActivityLevel_Unknown = -1,
		k_EDeviceActivityLevel_Idle = 0,						// No activity for the last 10 seconds
		k_EDeviceActivityLevel_UserInteraction = 1,				// Activity (movement or prox sensor) is happening now	
		k_EDeviceActivityLevel_UserInteraction_Timeout = 2,		// No activity for the last 0.5 seconds
		k_EDeviceActivityLevel_Standby = 3,						// Idle for at least 5 seconds (configurable in Settings -> Power Management)
	};


	/** VR controller button and axis IDs */
	enum EVRButtonId {
		k_EButton_System = 0,
		k_EButton_ApplicationMenu = 1,
		k_EButton_Grip = 2,
		k_EButton_DPad_Left = 3,
		k_EButton_DPad_Up = 4,
		k_EButton_DPad_Right = 5,
		k_EButton_DPad_Down = 6,
		k_EButton_A = 7,

		k_EButton_ProximitySensor = 31,

		k_EButton_Axis0 = 32,
		k_EButton_Axis1 = 33,
		k_EButton_Axis2 = 34,
		k_EButton_Axis3 = 35,
		k_EButton_Axis4 = 36,

		// aliases for well known controllers
		k_EButton_SteamVR_Touchpad = k_EButton_Axis0,
		k_EButton_SteamVR_Trigger = k_EButton_Axis1,

		k_EButton_Dashboard_Back = k_EButton_Grip,

		k_EButton_Max = 64
	};

	inline uint64_t ButtonMaskFromId(EVRButtonId id) { return 1ull << id; }

	/** used for controller button events */
	struct VREvent_Controller_t {
		uint32_t button; // EVRButtonId enum
	};


	/** used for simulated mouse events in overlay space */
	enum EVRMouseButton {
		VRMouseButton_Left = 0x0001,
		VRMouseButton_Right = 0x0002,
		VRMouseButton_Middle = 0x0004,
	};


	/** used for simulated mouse events in overlay space */
	struct VREvent_Mouse_t {
		float x, y; // co-ords are in GL space, bottom left of the texture is 0,0
		uint32_t button; // EVRMouseButton enum
	};

	/** used for simulated mouse wheel scroll in overlay space */
	struct VREvent_Scroll_t {
		float xdelta, ydelta; // movement in fraction of the pad traversed since last delta, 1.0 for a full swipe
		uint32_t repeatCount;
	};

	/** when in mouse input mode you can receive data from the touchpad, these events are only sent if the users finger
	is on the touchpad (or just released from it)
	**/
	struct VREvent_TouchPadMove_t {
		// true if the users finger is detected on the touch pad
		bool bFingerDown;

		// How long the finger has been down in seconds
		float flSecondsFingerDown;

		// These values indicate the starting finger position (so you can do some basic swipe stuff)
		float fValueXFirst;
		float fValueYFirst;

		// This is the raw sampled coordinate without deadzoning
		float fValueXRaw;
		float fValueYRaw;
	};

	/** notification related events. Details will still change at this point */
	struct VREvent_Notification_t {
		uint64_t ulUserValue;
		uint32_t notificationId;
	};

	/** Used for events about processes */
	struct VREvent_Process_t {
		uint32_t pid;
		uint32_t oldPid;
		bool bForced;
	};


	/** Used for a few events about overlays */
	struct VREvent_Overlay_t {
		uint64_t overlayHandle;
	};


	/** Used for a few events about overlays */
	struct VREvent_Status_t {
		uint32_t statusState; // EVRState enum
	};

	/** Used for keyboard events **/
	struct VREvent_Keyboard_t {
		char cNewInput[8];	// Up to 11 bytes of new input
		uint64_t uUserValue;	// Possible flags about the new input
	};

	struct VREvent_Ipd_t {
		float ipdMeters;
	};

	struct VREvent_Chaperone_t {
		uint64_t m_nPreviousUniverse;
		uint64_t m_nCurrentUniverse;
	};

	/** Not actually used for any events */
	struct VREvent_Reserved_t {
		uint64_t reserved0;
		uint64_t reserved1;
	};

	struct VREvent_PerformanceTest_t {
		uint32_t m_nFidelityLevel;
	};

	struct VREvent_SeatedZeroPoseReset_t {
		bool bResetBySystemMenu;
	};

	struct VREvent_Screenshot_t {
		uint32_t handle;
		uint32_t type;
	};

	struct VREvent_ScreenshotProgress_t {
		float progress;
	};

	struct VREvent_ApplicationLaunch_t {
		uint32_t pid;
		uint32_t unArgsHandle;
	};

	struct VREvent_EditingCameraSurface_t {
		uint64_t overlayHandle;
		uint32_t nVisualMode;
	};

	struct VREvent_MessageOverlay_t {
		uint32_t unVRMessageOverlayResponse; // vr::VRMessageOverlayResponse enum
	};

	struct VREvent_Property_t {
		PropertyContainerHandle_t container;
		ETrackedDeviceProperty prop;
	};

	/** NOTE!!! If you change this you MUST manually update openvr_interop.cs.py */
	typedef union {
		VREvent_Reserved_t reserved;
		VREvent_Controller_t controller;
		VREvent_Mouse_t mouse;
		VREvent_Scroll_t scroll;
		VREvent_Process_t process;
		VREvent_Notification_t notification;
		VREvent_Overlay_t overlay;
		VREvent_Status_t status;
		VREvent_Keyboard_t keyboard;
		VREvent_Ipd_t ipd;
		VREvent_Chaperone_t chaperone;
		VREvent_PerformanceTest_t performanceTest;
		VREvent_TouchPadMove_t touchPadMove;
		VREvent_SeatedZeroPoseReset_t seatedZeroPoseReset;
		VREvent_Screenshot_t screenshot;
		VREvent_ScreenshotProgress_t screenshotProgress;
		VREvent_ApplicationLaunch_t applicationLaunch;
		VREvent_EditingCameraSurface_t cameraSurface;
		VREvent_MessageOverlay_t messageOverlay;
		VREvent_Property_t property;
	} VREvent_Data_t;


#if defined(__linux__) || defined(__APPLE__) 
	// This structure was originally defined mis-packed on Linux, preserved for 
	// compatibility. 
#pragma pack( push, 4 )
#endif

	/** An event posted by the server to all running applications */
	struct VREvent_t {
		uint32_t eventType; // EVREventType enum
		TrackedDeviceIndex_t trackedDeviceIndex;
		float eventAgeSeconds;
		// event data must be the end of the struct as its size is variable
		VREvent_Data_t data;
	};

#if defined(__linux__) || defined(__APPLE__) 
#pragma pack( pop )
#endif

	/** The mesh to draw into the stencil (or depth) buffer to perform
	* early stencil (or depth) kills of pixels that will never appear on the HMD.
	* This mesh draws on all the pixels that will be hidden after distortion.
	*
	* If the HMD does not provide a visible area mesh pVertexData will be
	* NULL and unTriangleCount will be 0. */
	struct HiddenAreaMesh_t {
		const HmdVector2_t *pVertexData;
		uint32_t unTriangleCount;
	};


	enum EHiddenAreaMeshType {
		k_eHiddenAreaMesh_Standard = 0,
		k_eHiddenAreaMesh_Inverse = 1,
		k_eHiddenAreaMesh_LineLoop = 2,

		k_eHiddenAreaMesh_Max = 3,
	};


	/** Identifies what kind of axis is on the controller at index n. Read this type
	* with pVRSystem->Get( nControllerDeviceIndex, Prop_Axis0Type_Int32 + n );
	*/
	enum EVRControllerAxisType {
		k_eControllerAxis_None = 0,
		k_eControllerAxis_TrackPad = 1,
		k_eControllerAxis_Joystick = 2,
		k_eControllerAxis_Trigger = 3, // Analog trigger data is in the X axis
	};


	/** contains information about one axis on the controller */
	struct VRControllerAxis_t {
		float x; // Ranges from -1.0 to 1.0 for joysticks and track pads. Ranges from 0.0 to 1.0 for triggers were 0 is fully released.
		float y; // Ranges from -1.0 to 1.0 for joysticks and track pads. Is always 0.0 for triggers.
	};


	/** the number of axes in the controller state */
	static const uint32_t k_unControllerStateAxisCount = 5;


#if defined(__linux__) || defined(__APPLE__) 
	// This structure was originally defined mis-packed on Linux, preserved for 
	// compatibility. 
#pragma pack( push, 4 )
#endif

	/** Holds all the state of a controller at one moment in time. */
	struct VRControllerState001_t {
		// If packet num matches that on your prior call, then the controller state hasn't been changed since 
		// your last call and there is no need to process it
		uint32_t unPacketNum;

		// bit flags for each of the buttons. Use ButtonMaskFromId to turn an ID into a mask
		uint64_t ulButtonPressed;
		uint64_t ulButtonTouched;

		// Axis data for the controller's analog inputs
		VRControllerAxis_t rAxis[k_unControllerStateAxisCount];
	};
#if defined(__linux__) || defined(__APPLE__) 
#pragma pack( pop )
#endif


	typedef VRControllerState001_t VRControllerState_t;


	/** determines how to provide output to the application of various event processing functions. */
	enum EVRControllerEventOutputType {
		ControllerEventOutput_OSEvents = 0,
		ControllerEventOutput_VREvents = 1,
	};



	/** Collision Bounds Style */
	enum ECollisionBoundsStyle {
		COLLISION_BOUNDS_STYLE_BEGINNER = 0,
		COLLISION_BOUNDS_STYLE_INTERMEDIATE,
		COLLISION_BOUNDS_STYLE_SQUARES,
		COLLISION_BOUNDS_STYLE_ADVANCED,
		COLLISION_BOUNDS_STYLE_NONE,

		COLLISION_BOUNDS_STYLE_COUNT
	};

	/** Allows the application to customize how the overlay appears in the compositor */
	struct Compositor_OverlaySettings {
		uint32_t size; // sizeof(Compositor_OverlaySettings)
		bool curved, antialias;
		float scale, distance, alpha;
		float uOffset, vOffset, uScale, vScale;
		float gridDivs, gridWidth, gridScale;
		HmdMatrix44_t transform;
	};

	/** used to refer to a single VR overlay */
	typedef uint64_t VROverlayHandle_t;

	static const VROverlayHandle_t k_ulOverlayHandleInvalid = 0;

	/** Errors that can occur around VR overlays */
	enum EVROverlayError {
		VROverlayError_None = 0,

		VROverlayError_UnknownOverlay = 10,
		VROverlayError_InvalidHandle = 11,
		VROverlayError_PermissionDenied = 12,
		VROverlayError_OverlayLimitExceeded = 13, // No more overlays could be created because the maximum number already exist
		VROverlayError_WrongVisibilityType = 14,
		VROverlayError_KeyTooLong = 15,
		VROverlayError_NameTooLong = 16,
		VROverlayError_KeyInUse = 17,
		VROverlayError_WrongTransformType = 18,
		VROverlayError_InvalidTrackedDevice = 19,
		VROverlayError_InvalidParameter = 20,
		VROverlayError_ThumbnailCantBeDestroyed = 21,
		VROverlayError_ArrayTooSmall = 22,
		VROverlayError_RequestFailed = 23,
		VROverlayError_InvalidTexture = 24,
		VROverlayError_UnableToLoadFile = 25,
		VROverlayError_KeyboardAlreadyInUse = 26,
		VROverlayError_NoNeighbor = 27,
		VROverlayError_TooManyMaskPrimitives = 29,
		VROverlayError_BadMaskPrimitive = 30,
	};

	/** enum values to pass in to VR_Init to identify whether the application will
	* draw a 3D scene. */
	enum EVRApplicationType {
		VRApplication_Other = 0,		// Some other kind of application that isn't covered by the other entries 
		VRApplication_Scene = 1,		// Application will submit 3D frames 
		VRApplication_Overlay = 2,		// Application only interacts with overlays
		VRApplication_Background = 3,	// Application should not start SteamVR if it's not already running, and should not
										// keep it running if everything else quits.
										VRApplication_Utility = 4,		// Init should not try to load any drivers. The application needs access to utility
																		// interfaces (like IVRSettings and IVRApplications) but not hardware.
																		VRApplication_VRMonitor = 5,	// Reserved for vrmonitor
																		VRApplication_SteamWatchdog = 6,// Reserved for Steam
																		VRApplication_Bootstrapper = 7, // Start up SteamVR

																		VRApplication_Max
	};


	/** error codes for firmware */
	enum EVRFirmwareError {
		VRFirmwareError_None = 0,
		VRFirmwareError_Success = 1,
		VRFirmwareError_Fail = 2,
	};


	/** error codes for notifications */
	enum EVRNotificationError {
		VRNotificationError_OK = 0,
		VRNotificationError_InvalidNotificationId = 100,
		VRNotificationError_NotificationQueueFull = 101,
		VRNotificationError_InvalidOverlayHandle = 102,
		VRNotificationError_SystemWithUserValueAlreadyExists = 103,
	};


	/** error codes returned by Vr_Init */

	// Please add adequate error description to https://developer.valvesoftware.com/w/index.php?title=Category:SteamVRHelp
	enum EVRInitError {
		VRInitError_None = 0,
		VRInitError_Unknown = 1,

		VRInitError_Init_InstallationNotFound = 100,
		VRInitError_Init_InstallationCorrupt = 101,
		VRInitError_Init_VRClientDLLNotFound = 102,
		VRInitError_Init_FileNotFound = 103,
		VRInitError_Init_FactoryNotFound = 104,
		VRInitError_Init_InterfaceNotFound = 105,
		VRInitError_Init_InvalidInterface = 106,
		VRInitError_Init_UserConfigDirectoryInvalid = 107,
		VRInitError_Init_HmdNotFound = 108,
		VRInitError_Init_NotInitialized = 109,
		VRInitError_Init_PathRegistryNotFound = 110,
		VRInitError_Init_NoConfigPath = 111,
		VRInitError_Init_NoLogPath = 112,
		VRInitError_Init_PathRegistryNotWritable = 113,
		VRInitError_Init_AppInfoInitFailed = 114,
		VRInitError_Init_Retry = 115, // Used internally to cause retries to vrserver
		VRInitError_Init_InitCanceledByUser = 116, // The calling application should silently exit. The user canceled app startup
		VRInitError_Init_AnotherAppLaunching = 117,
		VRInitError_Init_SettingsInitFailed = 118,
		VRInitError_Init_ShuttingDown = 119,
		VRInitError_Init_TooManyObjects = 120,
		VRInitError_Init_NoServerForBackgroundApp = 121,
		VRInitError_Init_NotSupportedWithCompositor = 122,
		VRInitError_Init_NotAvailableToUtilityApps = 123,
		VRInitError_Init_Internal = 124,
		VRInitError_Init_HmdDriverIdIsNone = 125,
		VRInitError_Init_HmdNotFoundPresenceFailed = 126,
		VRInitError_Init_VRMonitorNotFound = 127,
		VRInitError_Init_VRMonitorStartupFailed = 128,
		VRInitError_Init_LowPowerWatchdogNotSupported = 129,
		VRInitError_Init_InvalidApplicationType = 130,
		VRInitError_Init_NotAvailableToWatchdogApps = 131,
		VRInitError_Init_WatchdogDisabledInSettings = 132,
		VRInitError_Init_VRDashboardNotFound = 133,
		VRInitError_Init_VRDashboardStartupFailed = 134,
		VRInitError_Init_VRHomeNotFound = 135,
		VRInitError_Init_VRHomeStartupFailed = 136,
		VRInitError_Init_RebootingBusy = 137,
		VRInitError_Init_FirmwareUpdateBusy = 138,
		VRInitError_Init_FirmwareRecoveryBusy = 139,


		VRInitError_Driver_Failed = 200,
		VRInitError_Driver_Unknown = 201,
		VRInitError_Driver_HmdUnknown = 202,
		VRInitError_Driver_NotLoaded = 203,
		VRInitError_Driver_RuntimeOutOfDate = 204,
		VRInitError_Driver_HmdInUse = 205,
		VRInitError_Driver_NotCalibrated = 206,
		VRInitError_Driver_CalibrationInvalid = 207,
		VRInitError_Driver_HmdDisplayNotFound = 208,
		VRInitError_Driver_TrackedDeviceInterfaceUnknown = 209,
		// VRInitError_Driver_HmdDisplayNotFoundAfterFix = 210, // not needed: here for historic reasons
		VRInitError_Driver_HmdDriverIdOutOfBounds = 211,
		VRInitError_Driver_HmdDisplayMirrored = 212,

		VRInitError_IPC_ServerInitFailed = 300,
		VRInitError_IPC_ConnectFailed = 301,
		VRInitError_IPC_SharedStateInitFailed = 302,
		VRInitError_IPC_CompositorInitFailed = 303,
		VRInitError_IPC_MutexInitFailed = 304,
		VRInitError_IPC_Failed = 305,
		VRInitError_IPC_CompositorConnectFailed = 306,
		VRInitError_IPC_CompositorInvalidConnectResponse = 307,
		VRInitError_IPC_ConnectFailedAfterMultipleAttempts = 308,

		VRInitError_Compositor_Failed = 400,
		VRInitError_Compositor_D3D11HardwareRequired = 401,
		VRInitError_Compositor_FirmwareRequiresUpdate = 402,
		VRInitError_Compositor_OverlayInitFailed = 403,
		VRInitError_Compositor_ScreenshotsInitFailed = 404,
		VRInitError_Compositor_UnableToCreateDevice = 405,

		VRInitError_VendorSpecific_UnableToConnectToOculusRuntime = 1000,

		VRInitError_VendorSpecific_HmdFound_CantOpenDevice = 1101,
		VRInitError_VendorSpecific_HmdFound_UnableToRequestConfigStart = 1102,
		VRInitError_VendorSpecific_HmdFound_NoStoredConfig = 1103,
		VRInitError_VendorSpecific_HmdFound_ConfigTooBig = 1104,
		VRInitError_VendorSpecific_HmdFound_ConfigTooSmall = 1105,
		VRInitError_VendorSpecific_HmdFound_UnableToInitZLib = 1106,
		VRInitError_VendorSpecific_HmdFound_CantReadFirmwareVersion = 1107,
		VRInitError_VendorSpecific_HmdFound_UnableToSendUserDataStart = 1108,
		VRInitError_VendorSpecific_HmdFound_UnableToGetUserDataStart = 1109,
		VRInitError_VendorSpecific_HmdFound_UnableToGetUserDataNext = 1110,
		VRInitError_VendorSpecific_HmdFound_UserDataAddressRange = 1111,
		VRInitError_VendorSpecific_HmdFound_UserDataError = 1112,
		VRInitError_VendorSpecific_HmdFound_ConfigFailedSanityCheck = 1113,

		VRInitError_Steam_SteamInstallationNotFound = 2000,
	};

	enum EVRScreenshotType {
		VRScreenshotType_None = 0,
		VRScreenshotType_Mono = 1, // left eye only
		VRScreenshotType_Stereo = 2,
		VRScreenshotType_Cubemap = 3,
		VRScreenshotType_MonoPanorama = 4,
		VRScreenshotType_StereoPanorama = 5
	};

	enum EVRScreenshotPropertyFilenames {
		VRScreenshotPropertyFilenames_Preview = 0,
		VRScreenshotPropertyFilenames_VR = 1,
	};

	enum EVRTrackedCameraError {
		VRTrackedCameraError_None = 0,
		VRTrackedCameraError_OperationFailed = 100,
		VRTrackedCameraError_InvalidHandle = 101,
		VRTrackedCameraError_InvalidFrameHeaderVersion = 102,
		VRTrackedCameraError_OutOfHandles = 103,
		VRTrackedCameraError_IPCFailure = 104,
		VRTrackedCameraError_NotSupportedForThisDevice = 105,
		VRTrackedCameraError_SharedMemoryFailure = 106,
		VRTrackedCameraError_FrameBufferingFailure = 107,
		VRTrackedCameraError_StreamSetupFailure = 108,
		VRTrackedCameraError_InvalidGLTextureId = 109,
		VRTrackedCameraError_InvalidSharedTextureHandle = 110,
		VRTrackedCameraError_FailedToGetGLTextureId = 111,
		VRTrackedCameraError_SharedTextureFailure = 112,
		VRTrackedCameraError_NoFrameAvailable = 113,
		VRTrackedCameraError_InvalidArgument = 114,
		VRTrackedCameraError_InvalidFrameBufferSize = 115,
	};

	enum EVRTrackedCameraFrameType {
		VRTrackedCameraFrameType_Distorted = 0,			// This is the camera video frame size in pixels, still distorted.
		VRTrackedCameraFrameType_Undistorted,			// In pixels, an undistorted inscribed rectangle region without invalid regions. This size is subject to changes shortly.
		VRTrackedCameraFrameType_MaximumUndistorted,	// In pixels, maximum undistorted with invalid regions. Non zero alpha component identifies valid regions.
		MAX_CAMERA_FRAME_TYPES
	};

	typedef uint64_t TrackedCameraHandle_t;
#define INVALID_TRACKED_CAMERA_HANDLE	((vr::TrackedCameraHandle_t)0)

	struct CameraVideoStreamFrameHeader_t {
		EVRTrackedCameraFrameType eFrameType;

		uint32_t nWidth;
		uint32_t nHeight;
		uint32_t nBytesPerPixel;

		uint32_t nFrameSequence;

		TrackedDevicePose_t standingTrackedDevicePose;
	};

	// Screenshot types
	typedef uint32_t ScreenshotHandle_t;

	static const uint32_t k_unScreenshotHandleInvalid = 0;

#pragma pack( pop )

	// figure out how to import from the VR API dll
#if defined(_WIN32)

#ifdef VR_API_EXPORT
#define VR_INTERFACE extern "C" __declspec( dllexport )
#else
#define VR_INTERFACE extern "C" __declspec( dllimport )
#endif

#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)

#ifdef VR_API_EXPORT
#define VR_INTERFACE extern "C" __attribute__((visibility("default")))
#else
#define VR_INTERFACE extern "C" 
#endif

#else
#error "Unsupported Platform."
#endif


#if defined( _WIN32 )
#define VR_CALLTYPE __cdecl
#else
#define VR_CALLTYPE 
#endif

} // namespace vr

#endif // _INCLUDE_VRTYPES_H
