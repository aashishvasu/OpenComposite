#pragma once
#include "BaseCommon.h"

#include "Misc/json/json.h"
#include <string>
#include <map>
#include <vector>

typedef vr::EVRSkeletalTrackingLevel OOVR_EVRSkeletalTrackingLevel;

enum OOVR_EVRSkeletalReferencePose
{
	VRSkeletalReferencePose_BindPose = 0,
	VRSkeletalReferencePose_OpenHand,
	VRSkeletalReferencePose_Fist,
	VRSkeletalReferencePose_GripLimit
};

enum OOVR_EVRFinger
{
	VRFinger_Thumb = 0,
	VRFinger_Index,
	VRFinger_Middle,
	VRFinger_Ring,
	VRFinger_Pinky,
	VRFinger_Count
};

enum OOVR_EVRFingerSplay
{
	VRFingerSplay_Thumb_Index = 0,
	VRFingerSplay_Index_Middle,
	VRFingerSplay_Middle_Ring,
	VRFingerSplay_Ring_Pinky,
	VRFingerSplay_Count
};

enum OOVR_EVRSummaryType
{
	// The skeletal summary data will match the animated bone transforms for the action.
	VRSummaryType_FromAnimation = 0,

	// The skeletal summary data will include unprocessed data directly from the device when available.
	// This data is generally less latent than the data that is computed from the animations.
	VRSummaryType_FromDevice = 1,
};

enum OOVR_EVRInputStringBits
{
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

struct OOVR_InputSkeletalActionData_t
{
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
struct OOVR_VRSkeletalSummaryData_t
{
	/** The amount that each finger is 'curled' inwards towards the palm.  In the case of the thumb,
	* this represents how much the thumb is wrapped around the fist.
	* 0 means straight, 1 means fully curled */
	float	flFingerCurl[VRFinger_Count];

	/** The amount that each pair of adjacent fingers are separated.
	* 0 means the digits are touching, 1 means they are fully separated.
	*/
	float	flFingerSplay[VRFingerSplay_Count];
};

struct OOVR_InputBindingInfo_t
{
	char rchDevicePathName[128];
	char rchInputPathName[128];
	char rchModeName[128];
	char rchSlotName[128];
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
	virtual EVRInputError SetActionManifestPath(const char *pchActionManifestPath);

	/** Returns a handle for an action set. This handle is used for all performance-sensitive calls. */
	virtual EVRInputError GetActionSetHandle(const char *pchActionSetName, VRActionSetHandle_t *pHandle);

	/** Returns a handle for an action. This handle is used for all performance-sensitive calls. */
	virtual EVRInputError GetActionHandle(const char *pchActionName, VRActionHandle_t *pHandle);

	/** Returns a handle for any path in the input system. E.g. /user/hand/right */
	virtual EVRInputError GetInputSourceHandle(const char *pchInputSourcePath, VRInputValueHandle_t  *pHandle);

	// --------------- Reading action state ------------------- //

	/** Reads the current state into all actions. After this call, the results of Get*Action calls
	* will be the same until the next call to UpdateActionState. */
	virtual EVRInputError UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets, uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount);

	/** Reads the state of a digital action given its handle. This will return VRInputError_WrongType if the type of
	* action is something other than digital */
	virtual EVRInputError GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of an analog action given its handle. This will return VRInputError_WrongType if the type of
	* action is something other than analog */
	virtual EVRInputError GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a pose action given its handle. */
	virtual EVRInputError GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a pose action given its handle for the number of seconds relative to now. This
	* will generally be called with negative times from the fUpdateTime fields in other actions. */
	virtual EVRInputError GetPoseActionDataRelativeToNow(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a pose action given its handle. The returned values will match the values returned
	* by the last call to IVRCompositor::WaitGetPoses(). */
	virtual EVRInputError GetPoseActionDataForNextFrame(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);


	/** Reads the state of a skeletal action given its handle. */
	virtual EVRInputError GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of a skeletal action given its handle. */
	virtual EVRInputError GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t *pActionData, uint32_t unActionDataSize);


	// ---------------  Static Skeletal Data ------------------- //

	/** Reads the number of bones in skeleton associated with the given action */
	virtual EVRInputError GetBoneCount(VRActionHandle_t action, uint32_t* pBoneCount);

	/** Fills the given array with the index of each bone's parent in the skeleton associated with the given action */
	virtual EVRInputError GetBoneHierarchy(VRActionHandle_t action, VR_ARRAY_COUNT(unIndexArayCount) vr::BoneIndex_t* pParentIndices, uint32_t unIndexArayCount);

	/** Fills the given buffer with the name of the bone at the given index in the skeleton associated with the given action */
	virtual EVRInputError GetBoneName(VRActionHandle_t action, vr::BoneIndex_t nBoneIndex, VR_OUT_STRING() char* pchBoneName, uint32_t unNameBufferSize);

	/** Fills the given buffer with the transforms for a specific static skeletal reference pose */
	virtual EVRInputError GetSkeletalReferenceTransforms(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalReferencePose eReferencePose, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount);

	/** Reads the level of accuracy to which the controller is able to track the user to recreate a skeletal pose */
	virtual EVRInputError GetSkeletalTrackingLevel(VRActionHandle_t action, vr::EVRSkeletalTrackingLevel* pSkeletalTrackingLevel);

	// ---------------  Dynamic Skeletal Data ------------------- //

	/** Reads the state of the skeletal bone data associated with this action and copies it into the given buffer. */
	virtual EVRInputError GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of the skeletal bone data associated with this action and copies it into the given buffer. */
	virtual EVRInputError GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount);

	/** Reads summary information about the current pose of the skeleton associated with the given action.   */
	virtual EVRInputError GetSkeletalSummaryData(VRActionHandle_t action, EVRSummaryType eSummaryType, VRSkeletalSummaryData_t * pSkeletalSummaryData);

	// Same as above, but default to VRSummaryType_FromDevice (TODO check if this matches SteamVR). This was used in IVRInput_005
	virtual EVRInputError GetSkeletalSummaryData(VRActionHandle_t action, VRSkeletalSummaryData_t * pSkeletalSummaryData);

	/** Reads the state of the skeletal bone data in a compressed form that is suitable for
		* sending over the network. The required buffer size will never exceed ( sizeof(VR_BoneTransform_t)*boneCount + 2).
		* Usually the size will be much smaller. */
	virtual EVRInputError GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void *pvCompressedData, uint32_t unCompressedSize, uint32_t *punRequiredCompressedSize, VRInputValueHandle_t ulRestrictToDevice);

	/** Reads the state of the skeletal bone data in a compressed form that is suitable for
	* sending over the network. The required buffer size will never exceed ( sizeof(VR_BoneTransform_t)*boneCount + 2).
	* Usually the size will be much smaller. */
	virtual EVRInputError GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void *pvCompressedData, uint32_t unCompressedSize, uint32_t *punRequiredCompressedSize);

	/** Turns a compressed buffer from GetSkeletalBoneDataCompressed and turns it back into a bone transform array. */
	virtual EVRInputError DecompressSkeletalBoneData(void *pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace *peTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount);


	/** Turns a compressed buffer from GetSkeletalBoneDataCompressed and turns it back into a bone transform array. */
	virtual EVRInputError DecompressSkeletalBoneData(const void *pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace eTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount);

	// --------------- Haptics ------------------- //

	/** Triggers a haptic event as described by the specified action */
	virtual EVRInputError TriggerHapticVibrationAction(VRActionHandle_t action, float fStartSecondsFromNow, float fDurationSeconds, float fFrequency, float fAmplitude, VRInputValueHandle_t ulRestrictToDevice);

	// --------------- Action Origins ---------------- //

	/** Retrieve origin handles for an action */
	virtual EVRInputError GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle, VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t *originsOut, uint32_t originOutCount);

	/** Retrieves the name of the origin in the current language */
	virtual EVRInputError GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize);

	/** Retrieves the name of the origin in the current language. unStringSectionsToInclude is a bitfield of values in EVRInputStringBits that allows the
			application to specify which parts of the origin's information it wants a string for. */
	virtual EVRInputError GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize, int32_t unStringSectionsToInclude);

	/** Retrieves useful information for the origin of this action */
	virtual EVRInputError GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t *pOriginInfo, uint32_t unOriginInfoSize);

	/** Retrieves useful information about the bindings for an action */
	virtual EVRInputError GetActionBindingInfo(VRActionHandle_t action, OOVR_InputBindingInfo_t *pOriginInfo, uint32_t unBindingInfoSize, uint32_t unBindingInfoCount, uint32_t *punReturnedBindingInfoCount);

	/** Shows the current binding for the action in-headset */
	virtual EVRInputError ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle);

	/** Shows the current binding all the actions in the specified action sets */
	virtual EVRInputError ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets, uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount, VRInputValueHandle_t originToHighlight);

	// --------------- Legacy Input ------------------- //
	virtual bool IsUsingLegacyInput();

private:
	// Represents an action set. This is a set of controls that can be configured
	// independantly - as I understand it, these are to be used for different portions
	// of a game. You might have one action set for shooting, one for driving, and so on.
	// It also stores the usage mode, which controls how it should eventually be shown
	// in the binding editor (whether the user can bind controls seperately for each hand
	// or not).
	// See https://github.com/ValveSoftware/openvr/wiki/Action-manifest#action-sets
	struct ActionSet {
		std::string name;
		std::string usage;
	};

	struct ActionSource {
		std::string sourceType;
		std::string sourceMode;
		std::string sourcePath;
		std::string sourceDevice;
		std::string actionSetName;
		std::string parameterSubMode;
		double sourceParametersActivateThreshold = -1;
		double sourceParametersDeactivateThreshold = -1;
		bool leftState;
		bool rightState;
	};
	struct Action {
		std::string name;
		std::string type;
		VRInputValueHandle_t leftInputValue;
		VRInputValueHandle_t rightInputValue;
		std::vector<ActionSource *> leftActionSources;
		std::vector<ActionSource *> rightActionSources;
	};
	struct InputValue {
		std::string name;
		std::string type;
		vr::TrackedDeviceIndex_t trackedDeviceIndex;
		bool isSetControllerStateFromLastUpdate;
		bool isSetControllerState;
		vr::VRControllerState_t controllerStateFromLastUpdate;
		vr::VRControllerState_t controllerState;
		bool isConnected;
	};

	// these are helper methods to be used internally
	void ProcessInputSource(Json::Value inputJson, VRActionHandle_t actionHandle, std::string sourceType,
		std::string parameterSubMode, std::string actionSetName);

	void DetermineActionState(uint64_t buttonId, uint64_t buttonFlags, bool pressedButtonState,
		bool& masterPressedButtonState, vr::VRControllerAxis_t axis, double activateThreshold, double deactivateThreshold,
		bool& bState, bool& bChanged, bool& actionSourceDirectionState);

	void BuildActionSet(const ActionSet *);

	/** Retrieves useful information about the bindings for an action */
	void GetActionSourceBindingInfo(const Action *action, const ActionSource *src, OOVR_InputBindingInfo_t *result);

	std::map<std::string, Action *> _stringActionMap;
	std::map<std::string, ActionSet *> _stringActionSetMap;
	std::map<std::string, InputValue *> _stringInputValueMap;
};
