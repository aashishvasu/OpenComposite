#pragma once

// Make this usable from DrvOpenXR to refresh the inputs after swapping sessions
// FIXME don't do that, it's ugly and slows down the build when modifying headers
#include "../BaseCommon.h"

#include "../Misc/Input/InteractionProfile.h"
#include "../Misc/json/json.h"
#include <map>
#include <string>
#include <vector>

typedef vr::EVRSkeletalTrackingLevel OOVR_EVRSkeletalTrackingLevel;

enum OOVR_EVRSkeletalReferencePose {
	VRSkeletalReferencePose_BindPose = 0,
	VRSkeletalReferencePose_OpenHand,
	VRSkeletalReferencePose_Fist,
	VRSkeletalReferencePose_GripLimit
};

enum OOVR_EVRFinger {
	VRFinger_Thumb = 0,
	VRFinger_Index,
	VRFinger_Middle,
	VRFinger_Ring,
	VRFinger_Pinky,
	VRFinger_Count
};

enum OOVR_EVRFingerSplay {
	VRFingerSplay_Thumb_Index = 0,
	VRFingerSplay_Index_Middle,
	VRFingerSplay_Middle_Ring,
	VRFingerSplay_Ring_Pinky,
	VRFingerSplay_Count
};

enum OOVR_EVRSummaryType {
	// The skeletal summary data will match the animated bone transforms for the action.
	VRSummaryType_FromAnimation = 0,

	// The skeletal summary data will include unprocessed data directly from the device when available.
	// This data is generally less latent than the data that is computed from the animations.
	VRSummaryType_FromDevice = 1,
};

enum OOVR_EVRInputStringBits {
	VRInputString_Hand = 0x01,
	VRInputString_ControllerType = 0x02,
	VRInputString_InputSource = 0x04,

	VRInputString_All = 0xFFFFFFFF
};

struct OOVR_InputAnalogActionData_t {
	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;

	// The origin that caused this action's current state
	vr::VRInputValueHandle_t activeOrigin;

	// The current state of this action; will be delta updates for mouse actions
	float x, y, z;

	// Deltas since the previous call to UpdateActionState()
	float deltaX, deltaY, deltaZ;

	// Time relative to now when this event happened. Will be negative to indicate a past time.
	float fUpdateTime;
};

struct OOVR_InputDigitalActionData_t {
	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;

	// The origin that caused this action's current state
	vr::VRInputValueHandle_t activeOrigin;

	// The current state of this action; will be true if currently pressed
	bool bState;

	// This is true if the state has changed since the last frame
	bool bChanged;

	// Time relative to now when this event happened. Will be negative to indicate a past time.
	float fUpdateTime;
};

struct OOVR_InputPoseActionData_t {
	// Whether or not this action is currently available to be bound in the active action set
	bool bActive;

	// The origin that caused this action's current state
	vr::VRInputValueHandle_t activeOrigin;

	// The current state of this action
	vr::TrackedDevicePose_t pose;
};

enum OOVR_EVRSkeletalTransformSpace {
	VRSkeletalTransformSpace_Model = 0,
	VRSkeletalTransformSpace_Parent = 1,
};

enum OOVR_EVRInputFilterCancelType {
	VRInputFilterCancel_Timers = 0,
	VRInputFilterCancel_Momentum = 1,
};

struct OOVR_InputSkeletalActionData_t {
	/** Whether or not this action is currently available to be bound in the active action set */
	bool bActive;

	/** The origin that caused this action's current state */
	vr::VRInputValueHandle_t activeOrigin;
};

struct OOVR_InputOriginInfo_t {
	vr::VRInputValueHandle_t devicePath;
	vr::TrackedDeviceIndex_t trackedDeviceIndex;
	char rchRenderModelComponentName[128];
};

struct OOVR_VRActiveActionSet_t {
	/** This is the handle of the action set to activate for this frame. */
	vr::VRActionSetHandle_t ulActionSet;

	/** This is the handle of a device path that this action set should be active for. To
	* activate for all devices, set this to k_ulInvalidInputValueHandle. */
	vr::VRInputValueHandle_t ulRestrictedToDevice;

	/** The action set to activate for all devices other than ulRestrictedDevice. If
	* ulRestrictedToDevice is set to k_ulInvalidInputValueHandle, this parameter is
	* ignored. */
	vr::VRActionSetHandle_t ulSecondaryActionSet;

	// This field is ignored
	uint32_t unPadding;

	/** The priority of this action set relative to other action sets. Any inputs
	* bound to a source (e.g. trackpad, joystick, trigger) will disable bindings in
	* other active action sets with a smaller priority. */
	int32_t nPriority;
};

/** Contains summary information about the current skeletal pose */
struct OOVR_VRSkeletalSummaryData_t {
	/** The amount that each finger is 'curled' inwards towards the palm.  In the case of the thumb,
	* this represents how much the thumb is wrapped around the fist.
	* 0 means straight, 1 means fully curled */
	float flFingerCurl[VRFinger_Count];

	/** The amount that each pair of adjacent fingers are separated.
	* 0 means the digits are touching, 1 means they are fully separated.
	*/
	float flFingerSplay[VRFingerSplay_Count];
};

struct OOVR_InputBindingInfo_t {
	char rchDevicePathName[128]; // Corresponds to ActionSource.svrDevicePathName
	char rchInputPathName[128]; // Corresponds to ActionSource.svrInputPathName
	char rchModeName[128]; // Corresponds to ActionSource.mode in string form, eg 'button'
	char rchSlotName[128]; // Corresponds to ActionSource.slot

	/**
	 * The type of the physical input that the user actuates. For example a button bound to the thumbstick click
	 * returns 'joystick' and the trigger mapped as a digital button returns 'trigger'.
	 *
	 * The known values are:
	 * - joystick (the thumbstick)
	 * - trigger (the main trigger and the grip)
	 * - button (the X/Y/A/B buttons and the menu button)
	 *
	 * Note: no documentation I could find on this, found it by running SteamVR and noting these values down
	 *
	 * @author ZNix
	 * @since OpenVR 1.10.30
	 */
	char rchInputSourceType[32];
};

class BaseInput {
public:
	typedef vr::VRInputValueHandle_t VRInputValueHandle_t;
	typedef vr::EVRInputError EVRInputError;
	typedef vr::VRActionSetHandle_t VRActionSetHandle_t;
	typedef vr::VRActionHandle_t VRActionHandle_t;
	typedef vr::ETrackingUniverseOrigin ETrackingUniverseOrigin;
	typedef vr::EVRSkeletalMotionRange EVRSkeletalMotionRange;
	typedef vr::VRBoneTransform_t VRBoneTransform_t;
	typedef OOVR_VRActiveActionSet_t VRActiveActionSet_t;
	typedef OOVR_InputDigitalActionData_t InputDigitalActionData_t;
	typedef OOVR_InputAnalogActionData_t InputAnalogActionData_t;
	typedef OOVR_InputPoseActionData_t InputPoseActionData_t;
	typedef OOVR_InputSkeletalActionData_t InputSkeletalActionData_t;
	typedef OOVR_EVRSkeletalTransformSpace EVRSkeletalTransformSpace;
	typedef OOVR_InputOriginInfo_t InputOriginInfo_t;
	typedef OOVR_EVRSkeletalReferencePose EVRSkeletalReferencePose;
	typedef OOVR_EVRSummaryType EVRSummaryType;
	typedef OOVR_VRSkeletalSummaryData_t VRSkeletalSummaryData_t;

	// ---------------  Handle management   --------------- //

	/** Sets the path to the action manifest JSON file that is used by this application. If this information
	* was set on the Steam partner site, calls to this function are ignored. If the Steam partner site
	* setting and the path provided by this call are different, VRInputError_MismatchedActionManifest is returned.
	* This call must be made before the first call to UpdateActionState or IVRSystem::PollNextEvent. */
	virtual EVRInputError SetActionManifestPath(const char* pchActionManifestPath);

	/** Returns a handle for an action set. This handle is used for all performance-sensitive calls. */
	virtual EVRInputError GetActionSetHandle(const char* pchActionSetName, VRActionSetHandle_t* pHandle);

	/** Returns a handle for an action. This handle is used for all performance-sensitive calls. */
	virtual EVRInputError GetActionHandle(const char* pchActionName, VRActionHandle_t* pHandle);

	/** Returns a handle for any path in the input system. E.g. /user/hand/right */
	virtual EVRInputError GetInputSourceHandle(const char* pchInputSourcePath, VRInputValueHandle_t* pHandle);

	// --------------- Reading action state ------------------- //

	/** Reads the current state into all actions. After this call, the results of Get*Action calls
	* will be the same until the next call to UpdateActionState. */
	virtual EVRInputError UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets, uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount);

	/** Reads the state of a digital action given its handle. This will return VRInputError_WrongType if the type of
	* action is something other than digital */
	virtual EVRInputError GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of an analog action given its handle. This will return VRInputError_WrongType if the type of
	* action is something other than analog */
	virtual EVRInputError GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/**
	 * Reads the state of a pose action given its handle.
	 *
	 * Renamed to GetPoseActionDataRelativeToNow as of IVRInput_006.
	 */
	virtual EVRInputError GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a pose action given its handle for the number of seconds relative to now. This
	* will generally be called with negative times from the fUpdateTime fields in other actions. */
	virtual EVRInputError GetPoseActionDataRelativeToNow(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a pose action given its handle. The returned values will match the values returned
	* by the last call to IVRCompositor::WaitGetPoses(). */
	virtual EVRInputError GetPoseActionDataForNextFrame(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a skeletal action given its handle. */
	virtual EVRInputError GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a skeletal action given its handle. */
	virtual EVRInputError GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize);

	/** Returns the current dominant hand for the user for this application. This function will only return success for applications
	* which include "supports_dominant_hand_setting": true in their action manifests. The dominant hand will only change after
	* a call to UpdateActionState, and the action data returned after that point will use the new dominant hand. */
	virtual EVRInputError GetDominantHand(vr::ETrackedControllerRole* peDominantHand);

	/** Sets the dominant hand for the user for this application. */
	virtual EVRInputError SetDominantHand(vr::ETrackedControllerRole eDominantHand);

	// ---------------  Static Skeletal Data ------------------- //

	/** Reads the number of bones in skeleton associated with the given action */
	virtual EVRInputError GetBoneCount(VRActionHandle_t action, uint32_t* pBoneCount);

	/** Fills the given array with the index of each bone's parent in the skeleton associated with the given action */
	virtual EVRInputError GetBoneHierarchy(VRActionHandle_t action, VR_ARRAY_COUNT(unIndexArayCount) vr::BoneIndex_t* pParentIndices, uint32_t unIndexArayCount);

	/** Fills the given buffer with the name of the bone at the given index in the skeleton associated with the given action */
	virtual EVRInputError GetBoneName(VRActionHandle_t action, vr::BoneIndex_t nBoneIndex, VR_OUT_STRING() char* pchBoneName, uint32_t unNameBufferSize);

	/** Fills the given buffer with the transforms for a specific static skeletal reference pose */
	virtual EVRInputError GetSkeletalReferenceTransforms(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalReferencePose eReferencePose, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount);

	/** Reads the level of accuracy to which the controller is able to track the user to recreate a skeletal pose */
	virtual EVRInputError GetSkeletalTrackingLevel(VRActionHandle_t action, vr::EVRSkeletalTrackingLevel* pSkeletalTrackingLevel);

	// ---------------  Dynamic Skeletal Data ------------------- //

	/** Reads the state of the skeletal bone data associated with this action and copies it into the given buffer. */
	virtual EVRInputError GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of the skeletal bone data associated with this action and copies it into the given buffer. */
	virtual EVRInputError GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount);

	/** Reads summary information about the current pose of the skeleton associated with the given action.   */
	virtual EVRInputError GetSkeletalSummaryData(VRActionHandle_t action, EVRSummaryType eSummaryType, VRSkeletalSummaryData_t* pSkeletalSummaryData);

	// Same as above, but default to VRSummaryType_FromDevice (TODO check if this matches SteamVR). This was used in IVRInput_005
	virtual EVRInputError GetSkeletalSummaryData(VRActionHandle_t action, VRSkeletalSummaryData_t* pSkeletalSummaryData);

	/** Reads the state of the skeletal bone data in a compressed form that is suitable for
		* sending over the network. The required buffer size will never exceed ( sizeof(VR_BoneTransform_t)*boneCount + 2).
		* Usually the size will be much smaller. */
	virtual EVRInputError GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void* pvCompressedData, uint32_t unCompressedSize, uint32_t* punRequiredCompressedSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of the skeletal bone data in a compressed form that is suitable for
	* sending over the network. The required buffer size will never exceed ( sizeof(VR_BoneTransform_t)*boneCount + 2).
	* Usually the size will be much smaller. */
	virtual EVRInputError GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void* pvCompressedData, uint32_t unCompressedSize, uint32_t* punRequiredCompressedSize);

	/** Turns a compressed buffer from GetSkeletalBoneDataCompressed and turns it back into a bone transform array. */
	virtual EVRInputError DecompressSkeletalBoneData(void* pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace* peTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount);

	/** Turns a compressed buffer from GetSkeletalBoneDataCompressed and turns it back into a bone transform array. */
	virtual EVRInputError DecompressSkeletalBoneData(const void* pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace eTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount);

	// --------------- Haptics ------------------- //

	/** Triggers a haptic event as described by the specified action */
	virtual EVRInputError TriggerHapticVibrationAction(VRActionHandle_t action, float fStartSecondsFromNow, float fDurationSeconds, float fFrequency, float fAmplitude, VRInputValueHandle_t ulRestrictToDevice);

	// --------------- Action Origins ---------------- //

	/** Retrieve origin handles for an action */
	virtual EVRInputError GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle, VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t* originsOut, uint32_t originOutCount);

	/** Retrieves the name of the origin in the current language */
	virtual EVRInputError GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize);

	/** Retrieves the name of the origin in the current language. unStringSectionsToInclude is a bitfield of values in EVRInputStringBits that allows the
			application to specify which parts of the origin's information it wants a string for. */
	virtual EVRInputError GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize, int32_t unStringSectionsToInclude);

	/** Retrieves useful information for the origin of this action */
	virtual EVRInputError GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t* pOriginInfo, uint32_t unOriginInfoSize);

	/** Retrieves useful information about the bindings for an action */
	virtual EVRInputError GetActionBindingInfo(VRActionHandle_t action, OOVR_InputBindingInfo_t* bindingInfo, uint32_t unBindingInfoSize, uint32_t unBindingInfoCount, uint32_t* punReturnedBindingInfoCount);

	/** Shows the current binding for the action in-headset */
	virtual EVRInputError ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle);

	/** Shows the current binding all the actions in the specified action sets */
	virtual EVRInputError ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets, uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount, VRInputValueHandle_t originToHighlight);

	/** Use this to query what action on the component returned by GetOriginTrackedDeviceInfo would trigger this binding. */
	virtual EVRInputError GetComponentStateForBinding(const char* pchRenderModelName, const char* pchComponentName,
	    const OOVR_InputBindingInfo_t* pOriginInfo, uint32_t unBindingInfoSize, uint32_t unBindingInfoCount,
	    vr::RenderModel_ComponentState_t* pComponentState);

	// --------------- Legacy Input ------------------- //
	virtual bool IsUsingLegacyInput();

	// --------------- Utilities ------------------- //

	/** Opens the binding user interface. If no app key is provided it will use the key from the calling process.
	* If no set is provided it will open to the root of the app binding page. */
	virtual EVRInputError OpenBindingUI(const char* pchAppKey, VRActionSetHandle_t ulActionSetHandle, VRInputValueHandle_t ulDeviceHandle, bool bShowOnDesktop);

	/**
	 * Returns the variant set in the current bindings. If the binding doesn't include a variant setting, this function
	 * will return an empty string
	 */
	virtual EVRInputError GetBindingVariant(vr::VRInputValueHandle_t ulDevicePath, char* pchVariantArray, uint32_t unVariantArraySize);

public: // INTERNAL FUNCTIONS
	/**
	 * Bind all the inputs to the current OpenXR session. This must be called after swapping the session to keep
	 * the inputs working.
	 */
	void BindInputsForSession();

	/**
	 * Similar to setting the manifest, but doesn't actually load one. Equivalent to passing in a blank manifest.
	 *
	 * Used for games that don't use the input system.
	 */
	void LoadEmptyManifest();

	/**
	 * Update the input stuff, called by BaseSystem.
	 *
	 * Currently, it updates the OpenXR stuff if the game is running in legacy mode.
	 */
	void InternalUpdate();

	/**
	 * Called to implement BaseSystem::GetControllerState. The documentation for that reads:
	 *
	 * Fills the supplied struct with the current state of the controller. Returns false if the controller index
	 * is invalid.
	 */
	bool GetLegacyControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t* controllerState);

	void TriggerLegacyHapticPulse(vr::TrackedDeviceIndex_t controllerDeviceIndex, uint64_t durationNanos);

	void GetHandSpace(vr::TrackedDeviceIndex_t index, XrSpace& space);

private:
	enum class ActionRequirement {
		Suggested = 0, // default
		Mandatory,
		Optional,
	};

	enum class ActionType {
		Boolean = 0,
		Vector1,
		Vector2,
		Vector3,
		Vibration,
		Pose,
		Skeleton,
	};

	enum class ActionSetUsage {
		LeftRight = 0, // User can bind each side separately
		Single, // What's bound to one controller is bound to the other
		Hidden, // Not shown to the user
	};

	// Represents an action set. This is a set of controls that can be configured
	// independently - as I understand it, these are to be used for different portions
	// of a game. You might have one action set for shooting, one for driving, and so on.
	// It also stores the usage mode, which controls how it should eventually be shown
	// in the binding editor (whether the user can bind controls separately for each hand
	// or not).
	// See https://github.com/ValveSoftware/openvr/wiki/Action-manifest#action-sets
	struct ActionSet {
		std::string fullName; // Eg '/actions/main'
		std::string name; // Eg 'main'
		ActionSetUsage usage = ActionSetUsage::LeftRight;

		XrActionSet xr = XR_NULL_HANDLE;
	};

	enum class ActionSourceMode {
		// Some may be missing, there's no documentation on this that I could find
		BUTTON,
		DPAD,
		NONE, // What? Occurs in NMS, probably more experimentation is required
		JOYSTICK,
		TRIGGER,
	};

	/**
	 * Represents an action source, as declared in the default mappings JSON (NOT the actions manifest).
	 *
	 * This describes a specific control on a specific device (eg, the 'a' button on the left hand),
	 * the 'output' (physical mode of interaction, such as clicking a button) and which Action it maps to.
	 * <p/>
	 * TODO document how this maps to OpenXR.
	 * <p/>
	 * This is defined like so in the JSON (one of these could define many ActionSources if they had
	 * multiple other inputs aside from 'click'):
	 *
	 * @code
	 * {
     *   "inputs": {
     *     "click": {
     *       "output": "/actions/main/in/Test1"
     *     }
     *   },
     *   "mode": "button",
     *   "path": "/user/hand/left/input/trackpad"
     * }
     * @endcode
	 */
	// TODO delete this and move the documentation to OOVR_InputBindingInfo_t
	struct ActionSource {
		std::string svrPathName; // Full SteamVR path name for this input, eg '/user/hand/left/input/trackpad'
		std::string svrDevicePathName; // The device path from svrPathName, eg '/user/hand/left'
		std::string svrInputPathName; // The input path from svrPathName, eg '/input/trigger'
		ActionSourceMode mode; // The 'mode' that this input is operating in, such as to toggle a joystick between analogue and dpad mode

		// The 'slot' such as 'click' in the example above, defines what type of physical action the user must take to
		// trigger this (eg touch vs press for a capacitive button).
		// TODO convert this to an enum
		std::string slot;
	};

	struct Action {
		std::string fullName; // Full name as set in the JSON file, eg '/actions/main/in/Test1'
		std::string shortName; // The last part of the name, eg 'Test1'
		ActionRequirement requirement = ActionRequirement::Suggested;
		bool haptic = false;
		ActionType type = ActionType::Boolean;
		ActionSet* set = nullptr;
		std::string setName; // The name of the action set - set before we've enumerated the action sets, eg 'main'

		XrAction xr = XR_NULL_HANDLE;
	};

	struct InputValue {
	};

	bool hasLoadedActions = false;
	bool usingLegacyInput = false;
	std::map<std::string, std::unique_ptr<ActionSet> > actionSets;
	std::map<std::string, std::unique_ptr<Action> > actions;

	std::string bindingsPath;

	XrActionSet legacyInputsSet = XR_NULL_HANDLE;

	void LoadBindingsSet(const std::string& bindingsPath, const struct InteractionProfile&, std::vector<XrActionSuggestedBinding>& bindings);

	void AddLegacyBindings(InteractionProfile& profile, std::vector<XrActionSuggestedBinding>& bindings);

	/**
	 * Convert a tracked device index to 0=left 1=right -1=other
	 */
	static int DeviceIndexToHandId(vr::TrackedDeviceIndex_t idx);

	struct LegacyControllerActions {
		XrAction system; // Oculus button
		XrAction menu, menuTouch; // Upper button on touch controller - B/Y
		XrAction btnA, btnATouch; // Lower button on touch controller - A/X

		XrAction stickX, stickY, stickBtn, stickBtnTouch; // Axis0
		XrAction trigger, triggerTouch; // Axis1
		XrAction grip; // Axis2

		XrAction haptic;

		// Note: the 'grip' pose runs along the axis of the Touch controller, the 'aim' pose comes
		// straight out the front if you're holding it neutral. They correspond to the old Oculus
		// and SteamVR poses.
		XrAction gripPoseAction, aimPoseAction;
		XrSpace gripPoseSpace, aimPoseSpace;
	};
	LegacyControllerActions legacyControllers[2] = {};

	// Utility functions
	static Action* cast_AH(VRActionHandle_t);
	static ActionSet* cast_ASH(VRActionSetHandle_t);
};
