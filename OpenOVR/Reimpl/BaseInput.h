#pragma once

// Make this usable from DrvOpenXR to refresh the inputs after swapping sessions
// FIXME don't do that, it's ugly and slows down the build when modifying headers
#include "../BaseCommon.h"

#include "Drivers/Backend.h"
#include "Misc/json/json.h"
#include <array>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Misc/Input/InputData.h"
#include "Misc/Input/InteractionProfile.h"
#include "Misc/Input/LegacyControllerActions.h"

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
	// Ctor/dtor
	BaseInput();
	~BaseInput();

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
	 * If a manifest is already loaded, this does nothing. It should be called when a function from the legacy
	 * input system is used. That way, games that belatedly load their manifests won't be mis-identified as using
	 * the legacy system (previously this was called after the first frame).
	 *
	 * Used for games that don't use the input system.
	 */
	void LoadEmptyManifestIfRequired();

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

	bool AreActionsLoaded();

	// Gets a property from the current active interaction profile, if there is an active profile and if the property is known.
	// A hand type of HAND_NONE will grab an HMD property.
	template <typename T>
	std::optional<T> GetProperty(vr::ETrackedDeviceProperty property, ITrackedDevice::HandType hand)
	{
		return (activeProfile) ? activeProfile->GetProperty<T>(property, hand) : std::nullopt;
	}

	// Requests the current interaction profile from the runtime
	void UpdateInteractionProfile();

	/**
	 * Get a number that increments each time xrSyncActions is called. Can be used to check if a cached input value
	 * is current or not.
	 */
	inline uint64_t GetSyncSerial() const { return syncSerial; }

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
		~ActionSource(); // Must be defined non-inline to avoid it ending up in stubs.gen.cpp

		std::string svrPathName; // Full SteamVR path name for this input, eg '/user/hand/left/input/trackpad'
		std::string svrDevicePathName; // The device path from svrPathName, eg '/user/hand/left'
		std::string svrInputPathName; // The input path from svrPathName, eg '/input/trigger'
		ActionSourceMode mode; // The 'mode' that this input is operating in, such as to toggle a joystick between analogue and dpad mode

		// The 'slot' such as 'click' in the example above, defines what type of physical action the user must take to
		// trigger this (eg touch vs press for a capacitive button).
		// TODO convert this to an enum
		std::string slot;
	};

	struct DpadBindingInfo {
		// Possible dpad directions
		enum class Direction {
			NORTH,
			SOUTH,
			EAST,
			WEST,
			CENTER
		};

		// mapping from OpenVR name to Direction
		inline static const std::unordered_map<std::string, Direction> directionMap = {
			{ "north", Direction::NORTH },
			{ "east", Direction::EAST },
			{ "south", Direction::SOUTH },
			{ "west", Direction::WEST },
			{ "center", Direction::CENTER }
		};

		// Map from dpad binding parent names to their corresponding actions
		// An example parent name might be "righttrackpad-vive_controller"
		struct ParentActions {
			XrAction vectorAction = XR_NULL_HANDLE;
			XrAction clickAction = XR_NULL_HANDLE;
			XrAction touchAction = XR_NULL_HANDLE;
		};
		inline static std::unordered_map<std::string, ParentActions> parents;

		// Deadzone for dpad.
		static constexpr float dpadDeadzoneRadius = 0.2;

		// These are angles for the different dpad segments.
		// For example, the north dpad area exists between 45deg and 135deg, the east between -45deg and 45deg, etc
		// The actual units for the angles are in radians, but they are labeled as degrees since I find degrees more intuitive to understand for humans.
		static constexpr float angle45deg = math_pi / 4;
		static constexpr float angle135deg = 3 * math_pi / 4;

		// The direction for this binding.
		Direction direction = Direction::NORTH;

		// Is a click required for this binding
		// false implies that a touch is required instead
		bool click = false;

		// The previous state for the dpad binding.
		bool lastState = false;
	};

	struct Action {
	private:
		static constexpr size_t sources_size = 32;

	public:
		~Action(); // Must be defined non-inline to avoid it ending up in stubs.gen.cpp

		// Since the list of VirtualInputs cannot be copied (only moved) we might as well make the
		// whole struct non-copyable to make the error messages easier to follow.
		Action() = default;
		Action(const Action&) = delete;

		std::string fullName; // Full name as set in the JSON file, eg '/actions/main/in/Test1'
		std::string shortName; // The last part of the name, eg 'Test1'
		ActionRequirement requirement = ActionRequirement::Suggested;
		bool haptic = false;
		ActionType type = ActionType::Boolean;
		ActionSet* set = nullptr;
		std::string setName; // The name of the action set - set before we've enumerated the action sets, eg 'main'

		XrAction xr = XR_NULL_HANDLE;

		// If this is a skeletal action, what hand it's bound to - this is set in the actions
		// manifest itself, not a binding file.
		ITrackedDevice::HandType skeletalHand = ITrackedDevice::HAND_NONE;

		// The action sources (paths like /user/hand/left/input/select/click, specifying an output of a physical
		// control) this action is bound to. This is cached, and is updated by activeOriginFromSubaction.
		XrPath sources[sources_size] = {};
		std::string sourceNames[sources_size] = {};
		uint32_t sourcesCount = 0; // Number of sources in the above that are defined
		uint64_t nextSourcesUpdate = 0; // For caching, the next value of syncSerial this should update at

		// Only used in the case of Pose actions, this is the action space for each subaction path
		// The indexes match up with allSubactionPaths
		std::vector<XrSpace> actionSpaces;

		// Only used in float/vector actions, for calculating deltas
		struct {
			float x = 0;
			float y = 0;
			float z = 0;
		} previousState;

		// list of dpad directions to check
		// first member of the pair is the parent name, the second is the corresponding binding info
		using DpadGrouping = std::pair<std::string, DpadBindingInfo>;
		std::vector<DpadGrouping> dpadBindings;
	};

	enum class InputSource {
		INVALID,
		HAND_LEFT,
		HAND_RIGHT,
		// HMD and Gamepad can go in here if necessary
	};

	struct InputValueHandle {
		InputValueHandle();
		~InputValueHandle();

		/**
		 * The full path of this handle, eg /user/hand/left/input/select/value.
		 *
		 * This is what was passed to GetInputSourceHandle.
		 */
		std::string path;

		/**
		 * If this is a valid input source (eg in /user/hand/left or a child thereof) this specifies
		 * what that input source is.
		 */
		InputSource type = InputSource::INVALID;

		/**
		 * If this is a valid input source, this specifies the path of the input device it is mounted on as
		 * an OpenXR path atom. If it is not, this is XR_NULL_PATH.
		 *
		 * This will match one of the paths from allSubactionPaths.
		 *
		 * Eg /user/hand/left.
		 */
		XrPath devicePath = XR_NULL_PATH;

		/**
		 * A string version of #devicePath. Null corresponds to an empty string.
		 */
		std::string devicePathString;
	};

	using RegHandle = uint64_t;

	// For keeping track of string-lookup based stuff: actions, actionsets, and inputvaluehandles.
	// These basically serve two purposes: storing useful stuff that OpenComposite inserts, and associating
	//  a name the game made up and we don't support with a consistent handle.
	// Think of this as being used for something like XrPath: the application can convert a string into a
	//  handle and that handle must be both unique for that string, and constant across calls with the same
	//  string supplied.
	// Additionally, some magic strings that OpenComposite loads are associated with additional data.
	// These provide an opaque handle for a given string. If that string has an object associated with it then
	//  it's that pointer (to make debugging easier, since you just cast to inspect the contents) or a dummy
	//  value of a constant plus some randomisation otherwise.
	// Handle-to-object lookups are always done through an unsorted map (no pointer cases, just in case the
	//  memory behind the dummy values are somehow allocated and the performance cost should be very
	//  small), the handle being the pointer solely for debugging.
	template <typename T>
	class Registry {
	public:
		Registry(uint32_t _maxNameSize);
		~Registry();

		T* LookupItem(const std::string& name) const;
		T* LookupItem(RegHandle handle) const;
		RegHandle LookupHandle(const std::string& name);
		T* Initialise(const std::string& name, std::unique_ptr<T> value);
		std::vector<std::unique_ptr<T>>& GetItems() { return storage; }

		// Function for shortening or looking up a shortened version of a name (if it exists)
		// Necessary because OpenXR has defined limits on name lengths, while OpenVR appears to have no such limits
		std::string ShortenOrLookupName(const std::string& longName);

	private:
		// A map of names to handles, used in the common case of not-the-first call
		std::unordered_map<std::string, RegHandle> handlesByName;

		// Reverse-lookup for debugging
		std::unordered_map<RegHandle, std::string> namesByHandle;

		// Handles to their associated item, or nothing
		std::unordered_map<RegHandle, T*> itemsByHandle;

		// Also for bonus performance, name to handle directly. Access time is more
		// important than memory usage here, as there won't be many actual items (maybe
		// single-digit thousands at most).
		std::unordered_map<std::string, T*> itemsByName;

		// The storage for all the actual items
		std::vector<std::unique_ptr<T>> storage;

		// A map for names that are too long.
		// For actions, these are names longer than XR_MAX_ACTION_NAME_SIZE
		// For action sets, XR_MAX_ACTION_SET_NAME_SIZE
		std::unordered_map<std::string, std::string> longNames;

		// The maximum name size. Does not include the null terminator.
		const uint32_t maxNameSize;
	};

	// See GetSyncSerial
	uint64_t syncSerial = 0;

	bool hasLoadedActions = false;
	std::string loadedActionsPath;
	bool usingLegacyInput = false;
	std::vector<std::unique_ptr<InteractionProfile>> interactionProfiles;
	Registry<ActionSet> actionSets;
	Registry<Action> actions;
	bool allowSetDominantHand = false;

	vr::ETrackedControllerRole dominantHand = vr::TrackedControllerRole_RightHand;

	// TODO convert to Registry
	std::unordered_map<std::string, std::unique_ptr<InputValueHandle>> inputHandleRegistry;

	XrActionSet legacyInputsSet = XR_NULL_HANDLE;

	/**
	 * The list of subaction paths anything can be bound to - this basically just means 'everything' and contains
	 * both the left and right hand paths as defined in the OpenXR spec.
	 */
	std::vector<XrPath> allSubactionPaths;

	/**
	 * String version of allSubactionPaths.
	 */
	std::vector<std::string> allSubactionPathNames = {
		"/user/hand/left",
		"/user/hand/right",
	};

	void LoadBindingsSet(const InteractionProfile& profile, const std::string& bindingsPath);

	void CreateLegacyActions();

	/**
	 * Convert a tracked device index to 0=left 1=right -1=other
	 */
	static int DeviceIndexToHandId(vr::TrackedDeviceIndex_t idx);

	LegacyControllerActions legacyControllers[2] = {};

	// From https://github.com/ValveSoftware/openvr/wiki/Hand-Skeleton
	// Used as indexes into the skeleton output data
	enum HandSkeletonBone {
		eBone_Root = 0,
		eBone_Wrist,
		eBone_Thumb0,
		eBone_Thumb1,
		eBone_Thumb2,
		eBone_Thumb3,
		eBone_IndexFinger0,
		eBone_IndexFinger1,
		eBone_IndexFinger2,
		eBone_IndexFinger3,
		eBone_IndexFinger4,
		eBone_MiddleFinger0,
		eBone_MiddleFinger1,
		eBone_MiddleFinger2,
		eBone_MiddleFinger3,
		eBone_MiddleFinger4,
		eBone_RingFinger0,
		eBone_RingFinger1,
		eBone_RingFinger2,
		eBone_RingFinger3,
		eBone_RingFinger4,
		eBone_PinkyFinger0,
		eBone_PinkyFinger1,
		eBone_PinkyFinger2,
		eBone_PinkyFinger3,
		eBone_PinkyFinger4,
		eBone_Aux_Thumb,
		eBone_Aux_IndexFinger,
		eBone_Aux_MiddleFinger,
		eBone_Aux_RingFinger,
		eBone_Aux_PinkyFinger,
		eBone_Count
	};

	void ConvertHandModelSpace(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output);
	void ConvertHandParentSpace(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* out_transforms);

	XrHandTrackerEXT handTrackers[2] = { XR_NULL_HANDLE, XR_NULL_HANDLE };

	// Utility functions
	Action* cast_AH(VRActionHandle_t);
	ActionSet* cast_ASH(VRActionSetHandle_t);
	static InputValueHandle* cast_IVH(VRInputValueHandle_t);
	static ITrackedDevice* ivhToDev(VRInputValueHandle_t handle);
	VRInputValueHandle_t devToIVH(vr::TrackedDeviceIndex_t index);
	static bool checkRestrictToDevice(vr::VRInputValueHandle_t restrict, XrPath subactionPath);

	/**
	 * Get the 'activeOrigin' corresponding to a particular sub-action path on a given action.
	 *
	 * in OpenVR the application knows exactly what input source caused the action, while on OpenXR
	 * we only know what subaction caused it. So if you have two buttons on the same controller
	 * bound, we can't tell which one was pressed. Therefore this will pick one in an arbitrary but
	 * consistent manner.
	 */
	VRInputValueHandle_t activeOriginFromSubaction(Action* action, const char* subactionPath);

	/**
	 * Get the state for a digital action, which could be bound to a DPad action.
	 */
	XrResult getBooleanOrDpadData(Action& action, XrActionStateGetInfo* getInfo, XrActionStateBoolean* state);

	InteractionProfile* activeProfile = nullptr;

	friend InteractionProfile;
};
