#include "stdafx.h"
#define BASE_IMPL
#include "BaseInput.h"
#include <string>


#include "static_bases.gen.h"
#include "BaseClientCore.h"
#include "BaseSystem.h"
#include <fstream>
#include <codecvt>
#include <iostream>
#include "Drivers/Backend.h"
#include <map>
#include <thread>
Json::Value _actionManifest;
Json::Value _bindingsJson;

using namespace std;
using namespace vr;


// This is a duplicate from BaseClientCore.cpp
static bool ReadJson(wstring path, Json::Value &result) {
	ifstream in(path, ios::binary);
	if (in) {
		std::stringstream contents;
		contents << in.rdbuf();
		contents >> result;
		return true;
	}
	else {
		result = Json::Value(Json::ValueType::objectValue);
		return false;
	}
}

// Convert a UTF-8 string to a UTF-16 (wide) string
static std::wstring utf8to16(const std::string& t_str) {
	//setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(t_str);
}

static std::string dirnameOf(const std::string& fname) {
	size_t pos = fname.find_last_of("\\/");
	return (std::string::npos == pos)
		? ""
		: fname.substr(0, pos);
}

// Case-insensitively compares two strings
static bool iequals(const string& a, const string& b) {
	// from https://stackoverflow.com/a/4119881
	unsigned int sz = a.size();
	if (b.size() != sz)
		return false;
	for (unsigned int i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}


// ---

EVRInputError BaseInput::SetActionManifestPath(const char *pchActionManifestPath) {

	if ((pchActionManifestPath != NULL) && (pchActionManifestPath[0] == '\0')) // null or empty string
		return VRInputError_InvalidParam;

	// Put this in the applist, so it can be edited from the launcher
	std::shared_ptr<BaseClientCore> bcc = GetBaseClientCore();
	if (bcc)
		bcc->SetManifestPath(pchActionManifestPath);

	wstring actionManifestPath = utf8to16(pchActionManifestPath);
	ReadJson(actionManifestPath, _actionManifest);

	Json::Value jsonDefaultBindings = _actionManifest["default_bindings"];
	int oculusTouchIndex = -1;
	int riftIndex = -1;
	int genericIndex = -1;
	for (int index = 0; index < jsonDefaultBindings.size(); index++)
	{
		string controllerType = jsonDefaultBindings[index]["controller_type"].asCString();
		if (iequals(controllerType, "oculus_touch"))
			oculusTouchIndex = index;
		else if (iequals(controllerType, "rift"))
			riftIndex = index;
		else if (iequals(controllerType, "generic"))
			genericIndex = index;

	}

	// prioritize what we load:
	int index = 0;
	if (oculusTouchIndex != -1)
		index = oculusTouchIndex;
	else if (riftIndex != -1)
		index = riftIndex;
	else if (genericIndex != -1)
		index = genericIndex;

	string bindingUrl = jsonDefaultBindings[index]["binding_url"].asString();
	string bindingsPath = dirnameOf(pchActionManifestPath) + "/" + bindingUrl;

	wstring bindingsPathw = utf8to16(bindingsPath);
	ReadJson(bindingsPathw, _bindingsJson);

	return VRInputError_None;
}
EVRInputError BaseInput::GetActionSetHandle(const char *pchActionSetName, VRActionSetHandle_t *pHandle) {

	// check if key is in map and return it, add new keys to the map
	string pchActionSetNameString = pchActionSetName;
	std::transform(pchActionSetNameString.begin(), pchActionSetNameString.end(), pchActionSetNameString.begin(), ::tolower);

	if (_stringActionSetMap.find(pchActionSetNameString) != _stringActionSetMap.end())
	{
		// key found, return it
		ActionSet *existingActionSet = _stringActionSetMap[pchActionSetNameString];
		(*((ActionSet**)pHandle)) = existingActionSet;

		return VRInputError_None;
	}
	else // key not found
	{
		// create new ActionSet and insert/return handle

		Json::Value jsonActionSet = _actionManifest["action_sets"];
		if (!jsonActionSet.isNull())
		{
			for (int index = 0; index < jsonActionSet.size(); index++)
			{
				if (iequals(jsonActionSet[index]["name"].asCString(), pchActionSetName))
				{
					ActionSet *actionSet = new ActionSet();

					string name = jsonActionSet[index]["name"].asString();
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
					actionSet->name = name;

					actionSet->usage = jsonActionSet[index]["usage"].asString();

					(*((ActionSet**)pHandle)) = actionSet;
					_stringActionSetMap[pchActionSetNameString] = actionSet;
					return VRInputError_None;
				}
			}
		}
		else // no action_sets in json, setup default
		{
			ActionSet *actionSet = new ActionSet();
			actionSet->name = "/actions/demo";
			actionSet->usage = "leftright";

			(*((ActionSet**)pHandle)) = actionSet;
			_stringActionSetMap[pchActionSetNameString] = actionSet;
			return VRInputError_None;
		}
	}

	return VRInputError_NameNotFound;
}
EVRInputError BaseInput::GetActionHandle(const char *pchActionName, VRActionHandle_t *pHandle) {

	// check if key is in map and return it, add new keys to the map
	string pchActionNameString = pchActionName;
	std::transform(pchActionNameString.begin(), pchActionNameString.end(), pchActionNameString.begin(), ::tolower);

	if (_stringActionMap.find(pchActionNameString) != _stringActionMap.end())
	{
		// key found, return it
		Action *existingAction = _stringActionMap[pchActionNameString];
		(*((Action**)pHandle)) = existingAction;

		return VRInputError_None;
	}
	else // key not found
	{
		// create new Action and insert/return handle

		Json::Value jsonAction = _actionManifest["actions"];
		for (int index = 0; index < jsonAction.size(); index++)
		{
			if (iequals(jsonAction[index]["name"].asCString(), pchActionName))
			{
				Action *action = new Action();
				action->name = jsonAction[index]["name"].asString();
				action->type = jsonAction[index]["type"].asString();

				(*((Action**)pHandle)) = action;
				_stringActionMap[pchActionNameString] = action;

				return VRInputError_None;
			}
		}
	}

	return VRInputError_None;
}
EVRInputError BaseInput::GetInputSourceHandle(const char *pchInputSourcePath, VRInputValueHandle_t  *pHandle) {

	// check if key is in map and return it, add new keys to the map
	string pchInputSourcePathString = pchInputSourcePath;
	std::transform(pchInputSourcePathString.begin(), pchInputSourcePathString.end(), pchInputSourcePathString.begin(), ::tolower);

	if (_stringInputValueMap.find(pchInputSourcePathString) != _stringInputValueMap.end())
	{
		// key found, return it
		InputValue *existingInputValue = _stringInputValueMap[pchInputSourcePathString];
		(*((InputValue**)pHandle)) = existingInputValue;

		return VRInputError_None;
	}
	else // key not found
	{
		// create new InputValue and insert/return handle

		InputValue *inputValue = new InputValue();

		if (iequals(pchInputSourcePath, "/user/head"))
		{
			inputValue->trackedDeviceIndex = BackendManager::Instance().GetPrimaryHMD()->DeviceIndex();
			inputValue->name = "/user/head";
		}
		else if (iequals(pchInputSourcePath, "/user/hand/left"))
		{
			inputValue->trackedDeviceIndex = BaseSystem::leftHandIndex;
			inputValue->name = "/user/hand/left";
		}
		else if (iequals(pchInputSourcePath, "/user/hand/right"))
		{
			inputValue->trackedDeviceIndex = BaseSystem::rightHandIndex;
			inputValue->name = "/user/hand/right";
		}
		else // catch all for all other unrecognized devices, to prevent crashing:
		{
			inputValue->trackedDeviceIndex = BaseSystem::thirdTouchIndex;
			inputValue->name = pchInputSourcePathString;
		}
		//else if (iequals(pchInputSourcePath, "/user/gamepad"))
		//{
		//	inputValue->trackedDeviceIndex = BaseSystem::thirdTouchIndex; // is this correct?
		//	inputValue->name = "/user/gamepad";
		//}
		//else
		//{
		//	return VRInputError_InvalidHandle;
		//}

		(*((InputValue**)pHandle)) = inputValue;
		_stringInputValueMap[pchInputSourcePathString] = inputValue;
	}

	return VRInputError_None;
}

void BaseInput::BuildActionSet(const ActionSet *actionSet) {
	// update controller state for each action:
	Json::Value jsonPoses = _bindingsJson["bindings"][actionSet->name]["poses"];
	for (int index = 0; index < jsonPoses.size(); index++) {
		Json::Value jsonCurrentItem = jsonPoses[index];
		VRActionHandle_t actionHandle = vr::k_ulInvalidActionHandle;
		GetActionHandle(jsonCurrentItem["output"].asCString(), &actionHandle);
		if (actionHandle == vr::k_ulInvalidActionHandle)
			continue; // action does not exist for this source

		Action *action = (Action *) actionHandle;

		string path = jsonCurrentItem["path"].asString();

		VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;

		string left = "/user/hand/left";
		string right = "/user/hand/right";
		string pathLeftSubst = path.substr(0, left.size());
		string pathRightSubst = path.substr(0, right.size());
		if (iequals(pathLeftSubst, left)) {
			GetInputSourceHandle(left.c_str(), &inputValueHandle);
			action->leftInputValue = inputValueHandle;

		}
		if (iequals(pathRightSubst, right)) {
			GetInputSourceHandle(right.c_str(), &inputValueHandle);
			action->rightInputValue = inputValueHandle;

		}

	}

	Json::Value jsonHaptics = _bindingsJson["bindings"][actionSet->name]["haptics"];
	for (int index = 0; index < jsonHaptics.size(); index++) {
		Json::Value jsonCurrentItem = jsonHaptics[index];
		VRActionHandle_t actionHandle = vr::k_ulInvalidActionHandle;
		GetActionHandle(jsonCurrentItem["output"].asCString(), &actionHandle);
		if (actionHandle == vr::k_ulInvalidActionHandle)
			continue; // action does not exist for this source

		Action *action = (Action *) actionHandle;

		string path = jsonCurrentItem["path"].asString();

		VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;

		string left = "/user/hand/left";
		string right = "/user/hand/right";
		string pathLeftSubst = path.substr(0, left.size());
		string pathRightSubst = path.substr(0, right.size());
		if (iequals(pathLeftSubst, left)) {
			GetInputSourceHandle(left.c_str(), &inputValueHandle);
			action->leftInputValue = inputValueHandle;
		}
		if (iequals(pathRightSubst, right)) {
			GetInputSourceHandle(right.c_str(), &inputValueHandle);
			action->rightInputValue = inputValueHandle;
		}

	}


	Json::Value jsonSkeleton = _bindingsJson["bindings"][actionSet->name]["skeleton"];
	for (int index = 0; index < jsonSkeleton.size(); index++) {
		Json::Value jsonCurrentItem = jsonSkeleton[index];
		VRActionHandle_t actionHandle = vr::k_ulInvalidActionHandle;
		GetActionHandle(jsonCurrentItem["output"].asCString(), &actionHandle);
		if (actionHandle == vr::k_ulInvalidActionHandle)
			continue; // action does not exist for this source

		Action *action = (Action *) actionHandle;

		string path = jsonCurrentItem["path"].asString();

		VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;

		string left = "/user/hand/left";
		string right = "/user/hand/right";
		string pathLeftSubst = path.substr(0, left.size());
		string pathRightSubst = path.substr(0, right.size());
		if (iequals(pathLeftSubst, left)) {
			GetInputSourceHandle(left.c_str(), &inputValueHandle);
			action->leftInputValue = inputValueHandle;
		}
		if (iequals(pathRightSubst, right)) {
			GetInputSourceHandle(right.c_str(), &inputValueHandle);
			action->rightInputValue = inputValueHandle;
		}

	}


	Json::Value jsonChords = _bindingsJson["bindings"][actionSet->name]["chords"];
	for (int index = 0; index < jsonChords.size(); index++) {
		Json::Value jsonCurrentItem = jsonChords[index];
		VRActionHandle_t actionHandle = vr::k_ulInvalidActionHandle;
		GetActionHandle(jsonCurrentItem["output"].asCString(), &actionHandle);
		if (actionHandle == vr::k_ulInvalidActionHandle)
			continue; // action does not exist for this source

		Action *action = (Action *) actionHandle;

		string path = jsonCurrentItem["path"].asString();

		VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;

		string left = "/user/hand/left";
		string right = "/user/hand/right";
		string pathLeftSubst = path.substr(0, left.size());
		string pathRightSubst = path.substr(0, right.size());
		if (iequals(pathLeftSubst, left)) {
			GetInputSourceHandle(left.c_str(), &inputValueHandle);
			action->leftInputValue = inputValueHandle;
		}
		if (iequals(pathRightSubst, right)) {
			GetInputSourceHandle(right.c_str(), &inputValueHandle);
			action->rightInputValue = inputValueHandle;
		}

	}


	Json::Value jsonSources = _bindingsJson["bindings"][actionSet->name]["sources"];
	for (int index = 0; index < jsonSources.size(); index++) {
		Json::Value jsonCurrentSourceItem = jsonSources[index];
		VRActionHandle_t actionHandle = vr::k_ulInvalidActionHandle;
		string sourceType;
		string parameterSubMode = "";

		Json::Value jsonSubMode = jsonCurrentSourceItem["parameters"]["sub_mode"];
		if (!jsonSubMode.isNull())
			parameterSubMode = jsonSubMode.asCString();

		Json::Value jsonClickOutput = jsonCurrentSourceItem["inputs"]["click"]["output"];
		if (!jsonClickOutput.isNull()) {
			GetActionHandle(jsonClickOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "click";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonPositionOutput = jsonCurrentSourceItem["inputs"]["position"]["output"];
		if (!jsonPositionOutput.isNull()) {
			GetActionHandle(jsonPositionOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "position";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonTouchOutput = jsonCurrentSourceItem["inputs"]["touch"]["output"];
		if (!jsonTouchOutput.isNull()) {
			GetActionHandle(jsonTouchOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "touch";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonPullOutput = jsonCurrentSourceItem["inputs"]["pull"]["output"];
		if (!jsonPullOutput.isNull()) {
			GetActionHandle(jsonPullOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "pull";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonNorthOutput = jsonCurrentSourceItem["inputs"]["north"]["output"];
		if (!jsonNorthOutput.isNull()) {
			GetActionHandle(jsonNorthOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "north";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonSouthOutput = jsonCurrentSourceItem["inputs"]["south"]["output"];
		if (!jsonSouthOutput.isNull()) {
			GetActionHandle(jsonSouthOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "south";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonEastOutput = jsonCurrentSourceItem["inputs"]["east"]["output"];
		if (!jsonEastOutput.isNull()) {
			GetActionHandle(jsonEastOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "east";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}

		Json::Value jsonWestOutput = jsonCurrentSourceItem["inputs"]["west"]["output"];
		if (!jsonWestOutput.isNull()) {
			GetActionHandle(jsonWestOutput.asCString(), &actionHandle);
			if (actionHandle != vr::k_ulInvalidActionHandle) {
				sourceType = "west";

				ProcessInputSource(jsonCurrentSourceItem, actionHandle, sourceType, parameterSubMode, actionSet->name);
			}
		}
	}
}

bool actionSourcesBuilt = false;
EVRInputError BaseInput::UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets,
	uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount) {

	/** Reads the current state into all actions. After this call, the results of Get*Action calls
	* will be the same until the next call to UpdateActionState. */

	if (pSets->ulActionSet == vr::k_ulInvalidActionSetHandle)
	{
		pSets->ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
		pSets->ulSecondaryActionSet = vr::k_ulInvalidActionSetHandle;
		return VRInputError_InvalidHandle;
	}

	// build up action source bindings, once
	if (!actionSourcesBuilt)
	{
		// for now, we ignore the provided active action set
		//ActionSet *actionSet = (ActionSet*)pSets->ulActionSet;

		// instead, we will setup all available action sets
		for (auto it = _stringActionSetMap.begin(); it != _stringActionSetMap.end(); ++it) {
			string actionSetName = it->first;
			ActionSet *actionSet = it->second;

			BuildActionSet(actionSet);
		}

		actionSourcesBuilt = true;
	}

	// update controller input states
	string left = "/user/hand/left";
	const char* inputHandlePath = left.c_str();
	VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;
	GetInputSourceHandle(inputHandlePath, &inputValueHandle);
	InputValue *inputValue = (InputValue*)inputValueHandle;
	TrackedDeviceIndex_t trackedDeviceIndex = inputValue->trackedDeviceIndex;
	ITrackedDevice *device = BackendManager::Instance().GetDevice(trackedDeviceIndex);
	if (device != nullptr)
	{
		// Note that this forces dual-origin mode on the LibOVR driver (OculusDevice.cpp), but it seems
		// perfectly stable at this point.
		// Also note that to fix the input lag issue, passing TrackingStateType_Rendering into GetPose in
		// GetPoseActionData would have probably worked, but this is more along the lines of what SteamVR
		// probably does.
		bool success = device->GetControllerState(&inputValue->controllerState);
		device->GetPose(TrackingUniverseSeated, &inputValue->seatedPose, TrackingStateType_Rendering);
		device->GetPose(TrackingUniverseStanding, &inputValue->standingPose, TrackingStateType_Rendering);
		device->GetPose(TrackingUniverseRawAndUncalibrated, &inputValue->rawPose, TrackingStateType_Rendering);
		inputValue->isConnected = true;
	}
	else
	{
		inputValue->isConnected = false;
	}

	string right = "/user/hand/right";
	const char* inputHandlePathRight = right.c_str();
	VRInputValueHandle_t inputValueHandleRight = vr::k_ulInvalidInputValueHandle;
	GetInputSourceHandle(inputHandlePathRight, &inputValueHandleRight);
	InputValue *inputValueRight = (InputValue*)inputValueHandleRight;
	TrackedDeviceIndex_t trackedDeviceIndexRight = inputValueRight->trackedDeviceIndex;
	ITrackedDevice *deviceRight = BackendManager::Instance().GetDevice(trackedDeviceIndexRight);
	if (deviceRight != nullptr)
	{
		bool success = deviceRight->GetControllerState(&inputValueRight->controllerState);
		deviceRight->GetPose(TrackingUniverseSeated, &inputValueRight->seatedPose, TrackingStateType_Rendering);
		deviceRight->GetPose(TrackingUniverseStanding, &inputValueRight->standingPose, TrackingStateType_Rendering);
		deviceRight->GetPose(TrackingUniverseRawAndUncalibrated, &inputValueRight->rawPose, TrackingStateType_Rendering);
		inputValueRight->isConnected = true;
	}
	else
	{
		inputValueRight->isConnected = false;
	}
	return VRInputError_None;
}

// helper method:
void BaseInput::ProcessInputSource(Json::Value inputJson, VRActionHandle_t actionHandle, string sourceType,
	string parameterSubMode, string actionSetName) {

	Action *action = (Action*)actionHandle;
	VRInputValueHandle_t inputValueHandle = vr::k_ulInvalidInputValueHandle;
	string path = inputJson["path"].asString();

	string left = "/user/hand/left";
	string right = "/user/hand/right";
	string pathLeftSubst = path.substr(0, left.size());
	string pathRightSubst = path.substr(0, right.size());
	if (iequals(pathLeftSubst, left))
	{
		GetInputSourceHandle(left.c_str(), &inputValueHandle);
		action->leftInputValue = inputValueHandle;

		// add action source to action
		ActionSource *actionSource = new ActionSource();
		actionSource->sourceMode = inputJson["mode"].asString();
		actionSource->sourcePath = path;
		actionSource->sourceType = sourceType;
		actionSource->parameterSubMode = parameterSubMode;
		actionSource->actionSetName = actionSetName;
		Json::Value activateThreshold = inputJson["parameters"]["click_activate_threshold"];
		if (!activateThreshold.isNull())
			actionSource->sourceParametersActivateThreshold = stod(activateThreshold.asString());
		Json::Value deactivateThreshold = inputJson["parameters"]["click_deactivate_threshold"];
		if (!deactivateThreshold.isNull())
			actionSource->sourceParametersDeactivateThreshold = stod(deactivateThreshold.asString());

		action->leftActionSources.push_back(*&actionSource);

	}
	if (iequals(pathRightSubst, right))
	{
		GetInputSourceHandle(right.c_str(), &inputValueHandle);
		action->rightInputValue = inputValueHandle;

		// add action source to action
		ActionSource *actionSource = new ActionSource();
		actionSource->sourceMode = inputJson["mode"].asString();
		actionSource->sourcePath = path;
		actionSource->sourceType = sourceType;
		actionSource->parameterSubMode = parameterSubMode;
		actionSource->actionSetName = actionSetName;
		Json::Value activateThreshold = inputJson["parameters"]["click_activate_threshold"];
		if (!activateThreshold.isNull())
			actionSource->sourceParametersActivateThreshold = stod(activateThreshold.asString());
		Json::Value deactivateThreshold = inputJson["parameters"]["click_deactivate_threshold"];
		if (!deactivateThreshold.isNull())
			actionSource->sourceParametersDeactivateThreshold = stod(deactivateThreshold.asString());

		action->rightActionSources.push_back(*&actionSource);

	}
}

// helper method
void BaseInput::DetermineActionState(uint64_t buttonId, uint64_t buttonFlags, bool pressedButtonState,
	bool& masterPressedButtonState, vr::VRControllerAxis_t axis, double activateThreshold, double deactivateThreshold,
	bool& bState, bool& bChanged, bool& actionSourceDirectionState) {

	uint64_t button = buttonFlags & ((uint64_t)1 << buttonId);
	bool hasActivateThreshold = activateThreshold != -1;
	bool hasDeactivateThreshold = deactivateThreshold != -1;

	if ((button > 0L && !pressedButtonState) && (!hasActivateThreshold || (hasActivateThreshold && axis.x >= activateThreshold)))
	{
		// button activated
		bState |= masterPressedButtonState = true;
		bChanged = true;
		actionSourceDirectionState = bState;
	}
	else if ((button == 0L && pressedButtonState) && (!hasDeactivateThreshold || (hasDeactivateThreshold && axis.x <= deactivateThreshold)))
	{
		// button deactivated
		bState |= masterPressedButtonState = false;
		bChanged = true;
		actionSourceDirectionState = bState;
	}
	else
	{
		// button state unchanged
		bChanged |= actionSourceDirectionState ^ masterPressedButtonState;
		actionSourceDirectionState = masterPressedButtonState;
		bState |= actionSourceDirectionState;
	}

}

// variables that hold the current state of controller inputs:
bool _rightTriggerPressed = false;
bool _rightGripPressed = false;
bool _rightThumbPressed = false;
bool _aButtonPressed = false;
bool _bButtonPressed = false;
bool _leftTriggerPressed = false;
bool _leftGripPressed = false;
bool _leftThumbPressed = false;
bool _xButtonPressed = false;
bool _yButtonPressed = false;

bool _aButtonTouched = false;
bool _bButtonTouched = false;
bool _xButtonTouched = false;
bool _yButtonTouched = false;
bool _menuButtonTouched = false;
bool _rightGripTouched = false;
bool _leftGripTouched = false;
bool _rightTriggerTouched = false;
bool _leftTriggerTouched = false;
bool _rightThumbTouched = false;
bool _leftThumbTouched = false;

bool _rightJoystickEast = false;
bool _rightJoystickWest = false;
bool _rightJoystickNorth = false;
bool _rightJoystickSouth = false;
bool _leftJoystickEast = false;
bool _leftJoystickWest = false;
bool _leftJoystickNorth = false;
bool _leftJoystickSouth = false;

EVRInputError BaseInput::GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {

	if (action == vr::k_ulInvalidActionHandle)
	{
		pActionData->activeOrigin = vr::k_ulInvalidInputValueHandle;
		pActionData->bActive = false;
		return VRInputError_InvalidHandle;
	}

	Action *digitalAction = (Action*)action;

	bool validDigitalType = iequals(digitalAction->type, "boolean");
	if (!validDigitalType)
		return VRInputError_WrongType;


	// Note: how to determine which input changed most recently???

	bool bActive = false;
	InputValue *inputValue = nullptr;
	VRControllerAxis_t triggerAxis;
	VRControllerAxis_t gripAxis;
	VRControllerAxis_t thumbstickAxis;
	VRControllerAxis_t emptyAxis;
	emptyAxis.x = 0;
	emptyAxis.y = 0;
	VRControllerAxis_t axis;
	VRInputValueHandle_t activeOrigin = vr::k_ulInvalidInputValueHandle;
	uint64_t buttonPressedFlags = 0;
	uint64_t buttonTouchedFlags = 0;
	string name;
	bool performRight = true;
	vector<ActionSource*> actionSources;

	// ulRestrictToDevice may tell us input handle to look at if both inputs are available
	if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle)
	{
		if (ulRestrictToDevice == digitalAction->rightInputValue)
			performRight = true;
		else if (ulRestrictToDevice == digitalAction->leftInputValue)
			performRight = false;

	}

	if (performRight && digitalAction->rightInputValue != k_ulInvalidInputValueHandle)	// look at right controller first, if available
	{
		inputValue = (InputValue*)digitalAction->rightInputValue;
		activeOrigin = digitalAction->rightInputValue;
		actionSources = digitalAction->rightActionSources;
	}
	else if (digitalAction->leftInputValue != vr::k_ulInvalidInputValueHandle) // look at left controller, if available
	{
		inputValue = (InputValue*)digitalAction->leftInputValue;
		activeOrigin = digitalAction->leftInputValue;
		actionSources = digitalAction->leftActionSources;
	}
	else
	{
		return VRInputError_None; // left and right controllers both not found? b/c action is not yet configured...
	}

	buttonPressedFlags = inputValue->controllerState.ulButtonPressed;
	buttonTouchedFlags = inputValue->controllerState.ulButtonTouched;
	thumbstickAxis = inputValue->controllerState.rAxis[0];
	triggerAxis = inputValue->controllerState.rAxis[1];
	gripAxis = inputValue->controllerState.rAxis[2];
	name = inputValue->name;


	// use the initial remembered button press states for each action in loop,
	// this lets us properly OR all of the state changes
	bool rightTriggerPressed = _rightTriggerPressed;
	bool rightGripPressed = _rightGripPressed;
	bool rightThumbPressed = _rightThumbPressed;
	bool aButtonPressed = _aButtonPressed;
	bool bButtonPressed = _bButtonPressed;
	bool leftTriggerPressed = _leftTriggerPressed;
	bool leftGripPressed = _leftGripPressed;
	bool leftThumbPressed = _leftThumbPressed;
	bool xButtonPressed = _xButtonPressed;
	bool yButtonPressed = _yButtonPressed;

	bool aButtonTouched = _aButtonTouched;
	bool bButtonTouched = _bButtonTouched;
	bool xButtonTouched = _xButtonTouched;
	bool yButtonTouched = _yButtonTouched;
	bool menuButtonTouched = _menuButtonTouched;
	bool rightGripTouched = _rightGripTouched;
	bool leftGripTouched = _leftGripTouched;
	bool rightTriggerTouched = _rightTriggerTouched;
	bool leftTriggerTouched = _leftTriggerTouched;
	bool rightThumbTouched = _rightThumbTouched;
	bool leftThumbTouched = _leftThumbTouched;

	bool rightJoystickEast = _rightJoystickEast;
	bool rightJoystickWest = _rightJoystickWest;
	bool rightJoystickNorth = _rightJoystickNorth;
	bool rightJoystickSouth = _rightJoystickSouth;
	bool leftJoystickEast = _leftJoystickEast;
	bool leftJoystickWest = _leftJoystickWest;
	bool leftJoystickNorth = _leftJoystickNorth;
	bool leftJoystickSouth = _leftJoystickSouth;

	// loop through all sources for this action and determine action state
	bool bState = false;
	bool bChanged = false;
	for (auto actionSourcePtr = begin(actionSources); actionSourcePtr != end(actionSources); ++actionSourcePtr)
	{
		ActionSource *actionSource = (*actionSourcePtr);
		string sourcePath = actionSource->sourcePath;
		string actionSetName = actionSource->actionSetName;
		string actionName = digitalAction->name;
		bActive = false;
		string actionNameSubst = actionName.substr(actionSetName.size(), actionName.size() - actionSetName.size());
		string pathSubst = sourcePath.substr(name.size(), sourcePath.size() - name.size());
		bool isRight = name == "/user/hand/right";
		bool isLeft = name == "/user/hand/left";

		if (iequals(pathSubst, "/input/grip") && iequals(actionSource->sourceType, "click"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_Grip, buttonPressedFlags, rightGripPressed, _rightGripPressed,
					gripAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_Grip, buttonPressedFlags, leftGripPressed, _leftGripPressed,
					gripAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/trigger") && iequals(actionSource->sourceType, "click"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Trigger, buttonPressedFlags, rightTriggerPressed, _rightTriggerPressed,
					triggerAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Trigger, buttonPressedFlags, leftTriggerPressed, _leftTriggerPressed,
					triggerAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/trigger") && iequals(actionSource->sourceType, "touch"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Trigger, buttonTouchedFlags, rightTriggerTouched, _rightTriggerTouched,
					triggerAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Trigger, buttonTouchedFlags, leftTriggerTouched, _leftTriggerTouched,
					triggerAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/joystick") && iequals(actionSource->sourceMode, "dpad") && iequals(actionSource->sourceType, "east"))
		{
			// NOTE: I didn't want to change the DPAD button mappings in GetControllerState() in OculusDevice.cpp,
			// because I'm not sure of the repercussions and I don't want to break any existing functionality, but
			// I found the current button flags to be incorrect with my oculus touch thumbsticks (CV1).  The true mappings
			// are as follows, which I account for in this method:
			// right means up
			// up means right
			// left means down
			// down means left

			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Up, buttonPressedFlags, rightJoystickEast, _rightJoystickEast,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Up, buttonPressedFlags, leftJoystickEast, _leftJoystickEast,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/joystick") && iequals(actionSource->sourceMode, "dpad") && iequals(actionSource->sourceType, "west"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Down, buttonPressedFlags, rightJoystickWest, _rightJoystickWest,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Down, buttonPressedFlags, leftJoystickWest, _leftJoystickWest,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/joystick") && iequals(actionSource->sourceMode, "dpad") && iequals(actionSource->sourceType, "north"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Right, buttonPressedFlags, rightJoystickNorth, _rightJoystickNorth,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Right, buttonPressedFlags, leftJoystickNorth, _leftJoystickNorth,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/joystick") && iequals(actionSource->sourceMode, "dpad") && iequals(actionSource->sourceType, "south"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Left, buttonPressedFlags, rightJoystickSouth, _rightJoystickSouth,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_DPad_Left, buttonPressedFlags, leftJoystickSouth, _leftJoystickSouth,
					thumbstickAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/joystick") && iequals(actionSource->sourceType, "click"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Touchpad, buttonPressedFlags, rightThumbPressed, _rightThumbPressed,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Touchpad, buttonPressedFlags, leftThumbPressed, _leftThumbPressed,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/joystick") && iequals(actionSource->sourceType, "touch"))
		{
			if (isRight)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Touchpad, buttonTouchedFlags, rightThumbTouched, _rightThumbTouched,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
			}
			else if (isLeft)
			{
				DetermineActionState((uint64_t)EVRButtonId::k_EButton_SteamVR_Touchpad, buttonTouchedFlags, leftThumbTouched, _leftThumbTouched,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
			}
		}
		else if (iequals(pathSubst, "/input/a") && iequals(actionSource->sourceType, "click"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_A, buttonPressedFlags, aButtonPressed, _aButtonPressed,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
		}
		else if (iequals(pathSubst, "/input/a") && iequals(actionSource->sourceType, "touch"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_A, buttonTouchedFlags, aButtonTouched, _aButtonTouched,
				emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
				bState, bChanged, actionSource->rightState);
		}
		else if (iequals(pathSubst, "/input/x") && iequals(actionSource->sourceType, "click"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_A, buttonPressedFlags, xButtonPressed, _xButtonPressed,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
		}
		else if (iequals(pathSubst, "/input/x") && iequals(actionSource->sourceType, "touch"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_A, buttonTouchedFlags, xButtonTouched, _xButtonTouched,
				emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
				bState, bChanged, actionSource->leftState);
		}
		else if (iequals(pathSubst, "/input/b") && iequals(actionSource->sourceType, "click"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_ApplicationMenu, buttonPressedFlags, bButtonPressed, _bButtonPressed,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->rightState);
		}
		else if (iequals(pathSubst, "/input/b") && iequals(actionSource->sourceType, "touch"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_ApplicationMenu, buttonTouchedFlags, bButtonTouched, _bButtonTouched,
				emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
				bState, bChanged, actionSource->rightState);
		}
		else if (iequals(pathSubst, "/input/y") && iequals(actionSource->sourceType, "click"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_ApplicationMenu, buttonPressedFlags, yButtonPressed, _yButtonPressed,
					emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
					bState, bChanged, actionSource->leftState);
		}
		else if (iequals(pathSubst, "/input/y") && iequals(actionSource->sourceType, "touch"))
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_ApplicationMenu, buttonTouchedFlags, yButtonTouched, _yButtonTouched,
				emptyAxis, actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
				bState, bChanged, actionSource->leftState);
		}
		else if (iequals(pathSubst, "/input/system") && isLeft)
		{
			DetermineActionState((uint64_t)EVRButtonId::k_EButton_System, buttonPressedFlags, menuButtonTouched, _menuButtonTouched,
				emptyAxis,actionSource->sourceParametersActivateThreshold, actionSource->sourceParametersDeactivateThreshold,
				bState, bChanged, actionSource->leftState);
		}
		else
		{
			// input not known for the current action - this input will need to be implemented. TBD
			continue;
		}

		bActive = true;

	}


	pActionData->activeOrigin = activeOrigin;
	pActionData->bActive = bActive;
	pActionData->bChanged = bChanged;
	pActionData->bState = bState;
	pActionData->fUpdateTime = 0; //todo: maybe get difference between statetime an current time ovr_GetTimeInSeconds

	return VRInputError_None;
}
EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {

	pActionData->bActive = false;
	if (action == vr::k_ulInvalidActionHandle)
	{
		pActionData->activeOrigin = vr::k_ulInvalidInputValueHandle;
		return VRInputError_InvalidHandle;
	}

	Action *analogAction = (Action*)action;

	bool validAnalogType = iequals(analogAction->type, "single") ||
		iequals(analogAction->type, "vector1") ||
		iequals(analogAction->type, "vector2") ||
		iequals(analogAction->type, "vector3");
	if (!validAnalogType)
		return VRInputError_WrongType;

	if (analogAction->leftInputValue == vr::k_ulInvalidInputValueHandle &&
		analogAction->rightInputValue == vr::k_ulInvalidInputValueHandle)
	{
		// If the action has no input, that means the action was defined in the action manifest but not defined in controller binding JSON.
		// This probably means the binding is optional and not set up, so we will mark it as inactive.
		pActionData->x = 0;
		pActionData->y = 0;
		pActionData->activeOrigin = vr::k_ulInvalidInputValueHandle;
		pActionData->bActive = false;

		return VRInputError_None;
	}

	// determine input based on action path:
	VRInputValueHandle_t activeOrigin;
	VRControllerAxis_t *axis = nullptr;
	string sourcePath;

	// ulRestrictToDevice may tell us input handle to look at if both inputs are available
	if (analogAction->leftInputValue != k_ulInvalidInputValueHandle &&
		analogAction->rightInputValue != k_ulInvalidInputValueHandle &&
		ulRestrictToDevice != vr::k_ulInvalidInputValueHandle)
	{
		activeOrigin = ulRestrictToDevice;
		InputValue *inputValue = (InputValue*)ulRestrictToDevice;
		axis = inputValue->controllerState.rAxis;
		if (ulRestrictToDevice == analogAction->leftInputValue)
			sourcePath = analogAction->leftActionSources[0]->sourcePath;
		else
			sourcePath = analogAction->rightActionSources[0]->sourcePath;

		if (sourcePath == "")
			return VRInputError_InvalidDevice;

	}
	else if (analogAction->leftInputValue != k_ulInvalidInputValueHandle)
	{
		activeOrigin = analogAction->leftInputValue;
		sourcePath = analogAction->leftActionSources[0]->sourcePath;
		InputValue *inputValueLeft = (InputValue*)analogAction->leftInputValue;
		axis = inputValueLeft->controllerState.rAxis;
	}
	else if (analogAction->rightInputValue != k_ulInvalidInputValueHandle)
	{
		activeOrigin = analogAction->rightInputValue;
		sourcePath = analogAction->rightActionSources[0]->sourcePath;
		InputValue *inputValueRight = (InputValue*)analogAction->rightInputValue;
		axis = inputValueRight->controllerState.rAxis;
	}


	// determine what data to output:
	InputValue *inputValue = (InputValue*)activeOrigin;
	string name = inputValue->name;
	string pathSubst = sourcePath.substr(name.size(), sourcePath.size() - name.size());
	VRControllerAxis_t analogData;
	if (iequals(pathSubst, "/input/trackpad") || iequals(pathSubst, "/input/joystick"))
	{
		analogData = axis[0]; // thumbstick
	}
	else if (iequals(pathSubst, "/input/trigger"))
	{
		analogData = axis[1]; // trigger
	}
	else if (iequals(pathSubst, "/input/grip"))
	{
		analogData = axis[2]; // grip
	}

	pActionData->x = analogData.x;
	pActionData->y = analogData.y;
	pActionData->activeOrigin = activeOrigin;
	pActionData->bActive = true;

	return VRInputError_None;
}
EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
	InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice) {

	//Todo: mke use of fPredictedSecondsFromNow

	if (action == vr::k_ulInvalidActionHandle)
	{
		pActionData->activeOrigin = vr::k_ulInvalidActionHandle;
		pActionData->pose.bPoseIsValid = false;
		pActionData->pose.bDeviceIsConnected = false;
		pActionData->bActive = false;
		return VRInputError_InvalidHandle;
	}

	Action *analogAction = (Action*)action;

	if (analogAction->leftInputValue == k_ulInvalidInputValueHandle &&
		analogAction->rightInputValue == k_ulInvalidInputValueHandle)
	{
		pActionData->activeOrigin = vr::k_ulInvalidActionHandle;
		pActionData->pose.bPoseIsValid = false;
		pActionData->pose.bDeviceIsConnected = false;
		pActionData->bActive = false;
		return VRInputError_None;
	}

	VRInputValueHandle_t activeOrigin = vr::k_ulInvalidInputValueHandle;
	TrackedDeviceIndex_t trackedDeviceIndex;

	InputValue *inputValue;
	// ulRestrictToDevice may tell us input handle to look at if both inputs are available
	if (analogAction->leftInputValue != k_ulInvalidInputValueHandle &&
		analogAction->rightInputValue != k_ulInvalidInputValueHandle &&
		ulRestrictToDevice != vr::k_ulInvalidInputValueHandle)
	{
		activeOrigin = ulRestrictToDevice;
		inputValue = (InputValue*)ulRestrictToDevice;
		trackedDeviceIndex = inputValue->trackedDeviceIndex;
	}
	else if (analogAction->leftInputValue != vr::k_ulInvalidInputValueHandle)
	{
		activeOrigin = analogAction->leftInputValue;
		inputValue = (InputValue*)analogAction->leftInputValue;
		trackedDeviceIndex = inputValue->trackedDeviceIndex;
	}
	else
	{
		activeOrigin = analogAction->rightInputValue;
		inputValue = (InputValue*)analogAction->rightInputValue;
		trackedDeviceIndex = inputValue->trackedDeviceIndex;
	}

	if (inputValue->isConnected)
	{
		if (eOrigin == TrackingUniverseSeated)
			pActionData->pose = inputValue->seatedPose;
		else if (eOrigin == TrackingUniverseStanding)
			pActionData->pose = inputValue->standingPose;
		else // TrackingUniverseRawAndUncalibrated
			pActionData->pose = inputValue->rawPose;

		pActionData->activeOrigin = activeOrigin;
		pActionData->bActive = true;
	}
	else // device not found, consider it disconnected
	{
		pActionData->activeOrigin = vr::k_ulInvalidActionHandle;
		pActionData->pose.bPoseIsValid = false;
		pActionData->pose.bDeviceIsConnected = false;
		pActionData->bActive = false;
	}

	return VRInputError_None;
}

EVRInputError BaseInput::GetPoseActionDataRelativeToNow(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice) {
	STUBBED();
}
EVRInputError BaseInput::GetPoseActionDataForNextFrame(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice) {

	return GetPoseActionData(action, eOrigin, 0, pActionData, unActionDataSize, ulRestrictToDevice);
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t *pActionData, uint32_t unActionDataSize) {

	if (action == vr::k_ulInvalidActionHandle)
	{
		pActionData->activeOrigin = vr::k_ulInvalidActionHandle;
		pActionData->bActive = false;
		return VRInputError_InvalidHandle;
	}

	Action *analogAction = (Action*)action;

	if (analogAction->leftInputValue == k_ulInvalidInputValueHandle &&
		analogAction->rightInputValue == k_ulInvalidInputValueHandle)
	{
		pActionData->activeOrigin = vr::k_ulInvalidActionHandle;
		pActionData->bActive = false;
		return VRInputError_InvalidDevice;
	}

	VRInputValueHandle_t activeOrigin = vr::k_ulInvalidInputValueHandle;
	TrackedDeviceIndex_t trackedDeviceIndex;

	if (analogAction->leftInputValue != vr::k_ulInvalidInputValueHandle)
	{
		activeOrigin = analogAction->leftInputValue;
		InputValue *inputValueLeft = (InputValue*)analogAction->leftInputValue;
		trackedDeviceIndex = inputValueLeft->trackedDeviceIndex;
	}
	else
	{
		activeOrigin = analogAction->rightInputValue;
		InputValue *inputValueRight = (InputValue*)analogAction->rightInputValue;
		trackedDeviceIndex = inputValueRight->trackedDeviceIndex;
	}

	ITrackedDevice *device = BackendManager::Instance().GetDevice(trackedDeviceIndex);
	if (device != nullptr)
	{
		pActionData->activeOrigin = activeOrigin;
		pActionData->bActive = false; // disable skeletal input for oculus touch
	}
	else // device not found, consider it disconnected
	{
		pActionData->activeOrigin = vr::k_ulInvalidActionHandle;
		pActionData->bActive = false;
	}

	return VRInputError_None;
}
EVRInputError BaseInput::GetBoneCount(VRActionHandle_t action, uint32_t* pBoneCount) {
	STUBBED();
}
EVRInputError BaseInput::GetBoneHierarchy(VRActionHandle_t action, VR_ARRAY_COUNT(unIndexArayCount) BoneIndex_t* pParentIndices, uint32_t unIndexArayCount) {
	STUBBED();
}
EVRInputError BaseInput::GetBoneName(VRActionHandle_t action, BoneIndex_t nBoneIndex, VR_OUT_STRING() char* pchBoneName, uint32_t unNameBufferSize) {
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalReferenceTransforms(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalReferencePose eReferencePose, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount) {
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalTrackingLevel(VRActionHandle_t action, EVRSkeletalTrackingLevel* pSkeletalTrackingLevel) {
	// STUBBED();
	// TODO implement properly, using the action
	// TODO check what SteamVR returns for this, and match it - it's not clear what it should be
	if(pSkeletalTrackingLevel)
		*pSkeletalTrackingLevel = VRSkeletalTracking_Estimated;

	return VRInputError_None;
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
	EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray,
	uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice) {
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
	EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount) {
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t action, EVRSummaryType eSummaryType, VRSkeletalSummaryData_t * pSkeletalSummaryData) {
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t action, VRSkeletalSummaryData_t * pSkeletalSummaryData) {
	return GetSkeletalSummaryData(action, VRSummaryType_FromDevice, pSkeletalSummaryData);
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
	EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void *pvCompressedData, uint32_t unCompressedSize,
	uint32_t *punRequiredCompressedSize, VRInputValueHandle_t ulRestrictToDevice) {

	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalMotionRange eMotionRange,
	VR_OUT_BUFFER_COUNT(unCompressedSize) void *pvCompressedData, uint32_t unCompressedSize, uint32_t *punRequiredCompressedSize) {
	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(void *pvCompressedBuffer, uint32_t unCompressedBufferSize,
	EVRSkeletalTransformSpace *peTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray,
	uint32_t unTransformArrayCount) {

	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(const void *pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace eTransformSpace,
	VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t *pTransformArray, uint32_t unTransformArrayCount) {
	STUBBED();
}

EVRInputError BaseInput::TriggerHapticVibrationAction(VRActionHandle_t action, float fStartSecondsFromNow, float fDurationSeconds,
	float fFrequency, float fAmplitude, VRInputValueHandle_t ulRestrictToDevice) {

	if (action == vr::k_ulInvalidActionHandle)
	{
		return VRInputError_InvalidHandle;
	}

	Action *vibrationAction = (Action*)action;

	InputValue *inputValue;

	// ulRestrictToDevice may tell us input handle to look at if both inputs are available
	if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle)
	{
		if (ulRestrictToDevice == vibrationAction->rightInputValue)
			inputValue = (InputValue*)vibrationAction->rightInputValue;
		else if (ulRestrictToDevice == vibrationAction->leftInputValue)
			inputValue = (InputValue*)vibrationAction->leftInputValue;

	}
	else if (vibrationAction->leftInputValue != k_ulInvalidInputValueHandle)
	{
		inputValue = (InputValue*)vibrationAction->leftInputValue;
	}
	else if (vibrationAction->rightInputValue != k_ulInvalidInputValueHandle)
	{
		inputValue = (InputValue*)vibrationAction->rightInputValue;
	}

	ITrackedDevice *device = BackendManager::Instance().GetDevice(inputValue->trackedDeviceIndex);

	// use async/threading to be non-blocking
	std::thread delay_thread([fStartSecondsFromNow, fDurationSeconds, device, fFrequency, fAmplitude]() {
		long startMillisecondsFromNow = round(fStartSecondsFromNow * 1000);
		long durationMilliseconds = round(fDurationSeconds * 1000);

		std::this_thread::sleep_for(std::chrono::milliseconds{ startMillisecondsFromNow }); // wait to start

		device->TriggerHapticVibrationAction(fFrequency, fAmplitude); // start non-buffered haptic

		std::this_thread::sleep_for(std::chrono::milliseconds{ durationMilliseconds }); // wait for duration

		device->TriggerHapticVibrationAction(0, 0); // stop haptic

	});
	delay_thread.detach();

	return VRInputError_None;
}
EVRInputError BaseInput::GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle,
	VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t *originsOut, uint32_t originOutCount) {

	// Retrieves the action sources for an action. If the action has more origins than will fit in the array,
	// only the number that will fit in the array are returned. If the action has fewer origins, the extra array
	// entries will be set to k_ulInvalidInputValueHandle

	uint32_t unActionDataSize;
	VRInputValueHandle_t ulRestrictToDevice = k_ulInvalidInputValueHandle;
	InputDigitalActionData_t *pActionData;

	Action *digitalAction = (Action *) digitalActionHandle;

	std::vector<VRInputValueHandle_t> vectorOriginOut;
	uint32_t count = originOutCount;

	// Note: right now the action source is going to be either left or right controller...
	// In the future, they should be split up to controller parts (ex: button a)
	if (digitalAction->leftInputValue != k_ulInvalidInputValueHandle && count > 0) {
		vectorOriginOut.push_back(digitalAction->leftInputValue);
		count--;
	}

	if (digitalAction->rightInputValue != k_ulInvalidInputValueHandle && count > 0) {
		vectorOriginOut.push_back(digitalAction->rightInputValue);
		count--;
	}

	while (count > 0) {
		vectorOriginOut.push_back(k_ulInvalidInputValueHandle);

		count--;
	}

	originsOut = &vectorOriginOut[0];

	return VRInputError_None;
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize) {
	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize,
	int32_t unStringSectionsToInclude) {
	STUBBED();
}
EVRInputError BaseInput::GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t *pOriginInfo, uint32_t unOriginInfoSize) {

	if (origin == vr::k_ulInvalidInputValueHandle)
	{
		pOriginInfo->devicePath = vr::k_ulInvalidInputValueHandle;
		pOriginInfo->trackedDeviceIndex = vr::k_unTrackedDeviceIndexInvalid;
		return VRInputError_InvalidHandle;
	}

	InputValue *input = (InputValue*)origin;
	TrackedDeviceIndex_t trackedDeviceIndex = input->trackedDeviceIndex;
	pOriginInfo->devicePath = origin;
	pOriginInfo->trackedDeviceIndex = trackedDeviceIndex;
	//pOriginInfo->rchRenderModelComponentName // todo

	return VRInputError_None;
}
EVRInputError BaseInput::ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle) {
	STUBBED();
}
EVRInputError BaseInput::ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets, uint32_t unSizeOfVRSelectedActionSet_t,
	uint32_t unSetCount, VRInputValueHandle_t originToHighlight) {

	STUBBED();
}

bool BaseInput::IsUsingLegacyInput() {
	STUBBED();
}
