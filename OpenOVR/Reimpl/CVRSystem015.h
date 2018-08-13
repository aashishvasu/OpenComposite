#pragma once
#include "OpenVR/interfaces/IVRSystem_015.h"
#include "BaseSystem.h"

using namespace vr;

class CVRSystem_015 : public IVRSystem_015::IVRSystem, public CVRCommon {
	CVR_GEN_IFACE();

	// Copied from IVRSystem, because MSVC made me.

	BaseSystem base;

public:

	// ------------------------------------
	// Display Methods
	// ------------------------------------

	/** Suggested size for the intermediate render target that the distortion pulls from. */
	INTERFACE_FUNC(void, GetRecommendedRenderTargetSize, uint32_t *pnWidth, uint32_t *pnHeight);

	/** The projection matrix for the specified eye */
	INTERFACE_FUNC(HmdMatrix44_t, GetProjectionMatrix, EVREye eEye, float fNearZ, float fFarZ);

	/** The components necessary to build your own projection matrix in case your
	* application is doing something fancy like infinite Z */
	INTERFACE_FUNC(void, GetProjectionRaw, EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom);

	/** Gets the result of the distortion function for the specified eye and input UVs. UVs go from 0,0 in
	* the upper left of that eye's viewport and 1,1 in the lower right of that eye's viewport.
	* Returns true for success. Otherwise, returns false, and distortion coordinates are not suitable. */
	INTERFACE_FUNC(bool, ComputeDistortion, EVREye eEye, float fU, float fV, DistortionCoordinates_t *pDistortionCoordinates);

	/** Returns the transform from eye space to the head space. Eye space is the per-eye flavor of head
	* space that provides stereo disparity. Instead of Model * View * Projection the sequence is Model * View * Eye^-1 * Projection.
	* Normally View and Eye^-1 will be multiplied together and treated as View in your application.
	*/
	INTERFACE_FUNC(HmdMatrix34_t, GetEyeToHeadTransform, EVREye eEye);

	/** Returns the number of elapsed seconds since the last recorded vsync event. This
	*	will come from a vsync timer event in the timer if possible or from the application-reported
	*   time if that is not available. If no vsync times are available the function will
	*   return zero for vsync time and frame counter and return false from the method. */
	INTERFACE_FUNC(bool, GetTimeSinceLastVsync, float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter);

	/** [D3D9 Only]
	* Returns the adapter index that the user should pass into CreateDevice to set up D3D9 in such
	* a way that it can go full screen exclusive on the HMD. Returns -1 if there was an error.
	*/
	INTERFACE_FUNC(int32_t, GetD3D9AdapterIndex);

	/** [D3D10/11 Only]
	* Returns the adapter index that the user should pass into EnumAdapters to create the device
	* and swap chain in DX10 and DX11. If an error occurs the index will be set to -1.
	*/
	INTERFACE_FUNC(void, GetDXGIOutputInfo, int32_t *pnAdapterIndex);

	// ------------------------------------
	// Display Mode methods
	// ------------------------------------

	/** Use to determine if the headset display is part of the desktop (i.e. extended) or hidden (i.e. direct mode). */
	INTERFACE_FUNC(bool, IsDisplayOnDesktop);

	/** Set the display visibility (true = extended, false = direct mode).  Return value of true indicates that the change was successful. */
	INTERFACE_FUNC(bool, SetDisplayVisibility, bool bIsVisibleOnDesktop);

	// ------------------------------------
	// Tracking Methods
	// ------------------------------------

	/** The pose that the tracker thinks that the HMD will be in at the specified number of seconds into the
	* future. Pass 0 to get the state at the instant the method is called. Most of the time the application should
	* calculate the time until the photons will be emitted from the display and pass that time into the method.
	*
	* This is roughly analogous to the inverse of the view matrix in most applications, though
	* many games will need to do some additional rotation or translation on top of the rotation
	* and translation provided by the head pose.
	*
	* For devices where bPoseIsValid is true the application can use the pose to position the device
	* in question. The provided array can be any size up to k_unMaxTrackedDeviceCount.
	*
	* Seated experiences should call this method with TrackingUniverseSeated and receive poses relative
	* to the seated zero pose. Standing experiences should call this method with TrackingUniverseStanding
	* and receive poses relative to the Chaperone Play Area. TrackingUniverseRawAndUncalibrated should
	* probably not be used unless the application is the Chaperone calibration tool itself, but will provide
	* poses relative to the hardware-specific coordinate system in the driver.
	*/
	INTERFACE_FUNC(void, GetDeviceToAbsoluteTrackingPose, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow, VR_ARRAY_COUNT(unTrackedDevicePoseArrayCount) TrackedDevicePose_t *pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount);

	/** Sets the zero pose for the seated tracker coordinate system to the current position and yaw of the HMD. After
	* ResetSeatedZeroPose all GetDeviceToAbsoluteTrackingPose calls that pass TrackingUniverseSeated as the origin
	* will be relative to this new zero pose. The new zero coordinate system will not change the fact that the Y axis
	* is up in the real world, so the next pose returned from GetDeviceToAbsoluteTrackingPose after a call to
	* ResetSeatedZeroPose may not be exactly an identity matrix.
	*
	* NOTE: This function overrides the user's previously saved seated zero pose and should only be called as the result of a user action.
	* Users are also able to set their seated zero pose via the OpenVR Dashboard.
	**/
	INTERFACE_FUNC(void, ResetSeatedZeroPose);

	/** Returns the transform from the seated zero pose to the standing absolute tracking system. This allows
	* applications to represent the seated origin to used or transform object positions from one coordinate
	* system to the other.
	*
	* The seated origin may or may not be inside the Play Area or Collision Bounds returned by IVRChaperone. Its position
	* depends on what the user has set from the Dashboard settings and previous calls to ResetSeatedZeroPose. */
	INTERFACE_FUNC(HmdMatrix34_t, GetSeatedZeroPoseToStandingAbsoluteTrackingPose);

	/** Returns the transform from the tracking origin to the standing absolute tracking system. This allows
	* applications to convert from raw tracking space to the calibrated standing coordinate system. */
	INTERFACE_FUNC(HmdMatrix34_t, GetRawZeroPoseToStandingAbsoluteTrackingPose);

	/** Get a sorted array of device indices of a given class of tracked devices (e.g. controllers).  Devices are sorted right to left
	* relative to the specified tracked device (default: hmd -- pass in -1 for absolute tracking space).  Returns the number of devices
	* in the list, or the size of the array needed if not large enough. */
	INTERFACE_FUNC(uint32_t, GetSortedTrackedDeviceIndicesOfClass, ETrackedDeviceClass eTrackedDeviceClass, VR_ARRAY_COUNT(unTrackedDeviceIndexArrayCount) TrackedDeviceIndex_t *punTrackedDeviceIndexArray, uint32_t unTrackedDeviceIndexArrayCount, TrackedDeviceIndex_t unRelativeToTrackedDeviceIndex = k_unTrackedDeviceIndex_Hmd);

	/** Returns the level of activity on the device. */
	INTERFACE_FUNC(EDeviceActivityLevel, GetTrackedDeviceActivityLevel, TrackedDeviceIndex_t unDeviceId);

	/** Convenience utility to apply the specified transform to the specified pose.
	*   This properly transforms all pose components, including velocity and angular velocity
	*/
	INTERFACE_FUNC(void, ApplyTransform, TrackedDevicePose_t *pOutputPose, const TrackedDevicePose_t *pTrackedDevicePose, const HmdMatrix34_t *pTransform);

	/** Returns the device index associated with a specific role, for example the left hand or the right hand. */
	INTERFACE_FUNC(TrackedDeviceIndex_t, GetTrackedDeviceIndexForControllerRole, ETrackedControllerRole unDeviceType);

	/** Returns the controller type associated with a device index. */
	INTERFACE_FUNC(ETrackedControllerRole, GetControllerRoleForTrackedDeviceIndex, TrackedDeviceIndex_t unDeviceIndex);

	// ------------------------------------
	// Property methods
	// ------------------------------------

	/** Returns the device class of a tracked device. If there has not been a device connected in this slot
	* since the application started this function will return TrackedDevice_Invalid. For previous detected
	* devices the function will return the previously observed device class.
	*
	* To determine which devices exist on the system, just loop from 0 to k_unMaxTrackedDeviceCount and check
	* the device class. Every device with something other than TrackedDevice_Invalid is associated with an
	* actual tracked device. */
	INTERFACE_FUNC(ETrackedDeviceClass, GetTrackedDeviceClass, TrackedDeviceIndex_t unDeviceIndex);

	/** Returns true if there is a device connected in this slot. */
	INTERFACE_FUNC(bool, IsTrackedDeviceConnected, TrackedDeviceIndex_t unDeviceIndex);

	/** Returns a bool property. If the device index is not valid or the property is not a bool type this function will return false. */
	INTERFACE_FUNC(bool, GetBoolTrackedDeviceProperty, TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L);

	/** Returns a float property. If the device index is not valid or the property is not a float type this function will return 0. */
	INTERFACE_FUNC(float, GetFloatTrackedDeviceProperty, TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L);

	/** Returns an int property. If the device index is not valid or the property is not a int type this function will return 0. */
	INTERFACE_FUNC(int32_t, GetInt32TrackedDeviceProperty, TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L);

	/** Returns a uint64 property. If the device index is not valid or the property is not a uint64 type this function will return 0. */
	INTERFACE_FUNC(uint64_t, GetUint64TrackedDeviceProperty, TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L);

	/** Returns a matrix property. If the device index is not valid or the property is not a matrix type, this function will return identity. */
	INTERFACE_FUNC(HmdMatrix34_t, GetMatrix34TrackedDeviceProperty, TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError *pError = 0L);

	/** Returns a string property. If the device index is not valid or the property is not a string type this function will
	* return 0. Otherwise it returns the length of the number of bytes necessary to hold this string including the trailing
	* null. Strings will always fit in buffers of k_unMaxPropertyStringSize characters. */
	INTERFACE_FUNC(uint32_t, GetStringTrackedDeviceProperty, TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, VR_OUT_STRING() char *pchValue, uint32_t unBufferSize, ETrackedPropertyError *pError = 0L);

	/** returns a string that corresponds with the specified property error. The string will be the name
	* of the error enum value for all valid error codes */
	INTERFACE_FUNC(const char *, GetPropErrorNameFromEnum, ETrackedPropertyError error);

	// ------------------------------------
	// Event methods
	// ------------------------------------

	/** Returns true and fills the event with the next event on the queue if there is one. If there are no events
	* this method returns false. uncbVREvent should be the size in bytes of the VREvent_t struct */
	INTERFACE_FUNC(bool, PollNextEvent, VREvent_t *pEvent, uint32_t uncbVREvent);

	/** Returns true and fills the event with the next event on the queue if there is one. If there are no events
	* this method returns false. Fills in the pose of the associated tracked device in the provided pose struct.
	* This pose will always be older than the call to this function and should not be used to render the device.
	uncbVREvent should be the size in bytes of the VREvent_t struct */
	INTERFACE_FUNC(bool, PollNextEventWithPose, ETrackingUniverseOrigin eOrigin, VREvent_t *pEvent, uint32_t uncbVREvent, TrackedDevicePose_t *pTrackedDevicePose);

	/** returns the name of an EVREvent enum value */
	INTERFACE_FUNC(const char *, GetEventTypeNameFromEnum, EVREventType eType);

	// ------------------------------------
	// Rendering helper methods
	// ------------------------------------

	/** Returns the hidden area mesh for the current HMD. The pixels covered by this mesh will never be seen by the user after the lens distortion is
	* applied based on visibility to the panels. If this HMD does not have a hidden area mesh, the vertex data and count will be NULL and 0 respectively.
	* This mesh is meant to be rendered into the stencil buffer (or into the depth buffer setting nearz) before rendering each eye's view.
	* This will improve performance by letting the GPU early-reject pixels the user will never see before running the pixel shader.
	* NOTE: Render this mesh with backface culling disabled since the winding order of the vertices can be different per-HMD or per-eye.
	* Setting the bInverse argument to true will produce the visible area mesh that is commonly used in place of full-screen quads. The visible area mesh covers all of the pixels the hidden area mesh does not cover.
	* Setting the bLineLoop argument will return a line loop of vertices in HiddenAreaMesh_t->pVertexData with HiddenAreaMesh_t->unTriangleCount set to the number of vertices.
	*/
	INTERFACE_FUNC(HiddenAreaMesh_t, GetHiddenAreaMesh, EVREye eEye, EHiddenAreaMeshType type = k_eHiddenAreaMesh_Standard);

	// ------------------------------------
	// Controller methods
	// ------------------------------------

	/** Fills the supplied struct with the current state of the controller. Returns false if the controller index
	* is invalid. */
	INTERFACE_FUNC(bool, GetControllerState, TrackedDeviceIndex_t unControllerDeviceIndex, VRControllerState_t *pControllerState, uint32_t unControllerStateSize);

	/** fills the supplied struct with the current state of the controller and the provided pose with the pose of
	* the controller when the controller state was updated most recently. Use this form if you need a precise controller
	* pose as input to your application when the user presses or releases a button. */
	INTERFACE_FUNC(bool, GetControllerStateWithPose, ETrackingUniverseOrigin eOrigin, TrackedDeviceIndex_t unControllerDeviceIndex, VRControllerState_t *pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t *pTrackedDevicePose);

	/** Trigger a single haptic pulse on a controller. After this call the application may not trigger another haptic pulse on this controller
	* and axis combination for 5ms. */
	INTERFACE_FUNC(void, TriggerHapticPulse, TrackedDeviceIndex_t unControllerDeviceIndex, uint32_t unAxisId, unsigned short usDurationMicroSec);

	/** returns the name of an EVRButtonId enum value */
	INTERFACE_FUNC(const char *, GetButtonIdNameFromEnum, EVRButtonId eButtonId);

	/** returns the name of an EVRControllerAxisType enum value */
	INTERFACE_FUNC(const char *, GetControllerAxisTypeNameFromEnum, EVRControllerAxisType eAxisType);

	/** Tells OpenVR that this process wants exclusive access to controller button states and button events. Other apps will be notified that
	* they have lost input focus with a VREvent_InputFocusCaptured event. Returns false if input focus could not be captured for
	* some reason. */
	INTERFACE_FUNC(bool, CaptureInputFocus);

	/** Tells OpenVR that this process no longer wants exclusive access to button states and button events. Other apps will be notified
	* that input focus has been released with a VREvent_InputFocusReleased event. */
	INTERFACE_FUNC(void, ReleaseInputFocus);

	/** Returns true if input focus is captured by another process. */
	INTERFACE_FUNC(bool, IsInputFocusCapturedByAnotherProcess);

	// ------------------------------------
	// Debug Methods
	// ------------------------------------

	/** Sends a request to the driver for the specified device and returns the response. The maximum response size is 32k,
	* but this method can be called with a smaller buffer. If the response exceeds the size of the buffer, it is truncated.
	* The size of the response including its terminating null is returned. */
	INTERFACE_FUNC(uint32_t, DriverDebugRequest, TrackedDeviceIndex_t unDeviceIndex, const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize);

	// ------------------------------------
	// Firmware methods
	// ------------------------------------

	/** Performs the actual firmware update if applicable.
	* The following events will be sent, if VRFirmwareError_None was returned: VREvent_FirmwareUpdateStarted, VREvent_FirmwareUpdateFinished
	* Use the properties Prop_Firmware_UpdateAvailable_Bool, Prop_Firmware_ManualUpdate_Bool, and Prop_Firmware_ManualUpdateURL_String
	* to figure our whether a firmware update is available, and to figure out whether its a manual update
	* Prop_Firmware_ManualUpdateURL_String should point to an URL describing the manual update process */
	INTERFACE_FUNC(EVRFirmwareError, PerformFirmwareUpdate, TrackedDeviceIndex_t unDeviceIndex);

	// ------------------------------------
	// Application life cycle methods
	// ------------------------------------

	/** Call this to acknowledge to the system that VREvent_Quit has been received and that the process is exiting.
	* This extends the timeout until the process is killed. */
	INTERFACE_FUNC(void, AcknowledgeQuit_Exiting);

	/** Call this to tell the system that the user is being prompted to save data. This
	* halts the timeout and dismisses the dashboard (if it was up). Applications should be sure to actually
	* prompt the user to save and then exit afterward, otherwise the user will be left in a confusing state. */
	INTERFACE_FUNC(void, AcknowledgeQuit_UserPrompt);
};
