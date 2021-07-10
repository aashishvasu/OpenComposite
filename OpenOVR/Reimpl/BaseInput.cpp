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

	// Load the action sets
	// This used to be done in GetActionSetHandle. This was fine (AFAIK) until HL:A came along (not that it's
	// their fault this time). Most games call ::GetActionSetHandle for each action set early on, while HL:A
	// doesn't. One issue of initialising this in GetActionSetHandle is that _stringActionSetMap is used
	// by UpdateActionState. If that's called before the action set handles have been taken, then that
	// action set isn't usable.
	// I'm not 100% certain this won't cause issues, though.
	Json::Value jsonActionSet = _actionManifest["action_sets"];
	if (!jsonActionSet.isNull()) {
		for (Json::Value& json : jsonActionSet) {
			ActionSet* actionSet = new ActionSet();

			string name = json["name"].asString();
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			actionSet->name = name;

			actionSet->usage = json["usage"].asString();

			_stringActionSetMap[name] = actionSet;
		}
	}

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
		Json::Value jsonActionSet = _actionManifest["action_sets"];
		if (jsonActionSet.isNull()) // no action_sets in json, setup default
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

	// Create a new action that doesn't appear in the manifest
	// Appears in No Man's Sky for quitgame
	// More experimentation may be required to figure out how SteamVR handles this
	auto *action = new Action();
	action->name = pchActionNameString;
	action->type = "<unset>";

	(*((Action**)pHandle)) = action;
	_stringActionMap[pchActionNameString] = action;

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
		inputValue->controllerStateFromLastUpdate = inputValue->controllerState; // keep track of previous controller state
		inputValue->isSetControllerStateFromLastUpdate = inputValue->isSetControllerState;
		bool success = device->GetControllerState(&inputValue->controllerState);
		inputValue->isSetControllerState = true;
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

		inputValueRight->controllerStateFromLastUpdate = inputValueRight->controllerState; // keep track of previous controller state
		inputValueRight->isSetControllerStateFromLastUpdate = inputValueRight->isSetControllerState;
		bool success = deviceRight->GetControllerState(&inputValueRight->controllerState);
		inputValueRight->isSetControllerState = true;
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

	// Find the physical type of the source. This depends on what physical control (eg, thumbstick or
	//  a button) it's mapped to. Put it here in a lambda since we need to know the path of
	//  the device (eg, /usr/hand/left) to find it out without doing ugly string parsing.
	auto findPhysicalType = [&path](const string& device) -> string {
		string input = path.substr(device.size());
		if (input == "/input/a" || input == "/input/b" || input == "/input/x" || input == "/input/y")
			return "button";
		if (input == "/input/system")
			return "button"; // Menu button
		else if (input == "/input/grip" || input == "/input/trigger")
			return "trigger";
		else if (input == "/input/joystick" || input == "/input/trackpad")
			// TODO should '/input/trackpad' return 'trackpad'?
			return "joystick";
		else
			// Should never happen, at least give us a descriptive error
			OOVR_ABORTF("BaseInput::ProcessInputSource unknown input name (phys-dev): '%s' for '%s'", input.c_str(), path.c_str());
	};

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
		actionSource->sourceDevice = pathLeftSubst; // TODO should it be left? Case might be different
		actionSource->sourcePhysicalType = findPhysicalType(left);
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
		actionSource->sourceDevice = pathRightSubst; // TODO should it be right? Case might be different
		actionSource->sourcePhysicalType = findPhysicalType(right);
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

	OOVR_FALSE_ABORT(sizeof(InputDigitalActionData_t) == unActionDataSize);

	// Initialise the action data to being invalid, in case we return without setting it
	memset(pActionData, 0, unActionDataSize);
	pActionData->activeOrigin = vr::k_ulInvalidInputValueHandle;
	pActionData->bActive = false;

	float functionCallTimeInSeconds = BackendManager::Instance().GetTimeInSeconds();

	if (action == vr::k_ulInvalidActionHandle)
	{
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

	float nowTimeInSeconds = BackendManager::Instance().GetTimeInSeconds();
	float fUpdateTime = functionCallTimeInSeconds - nowTimeInSeconds;

	pActionData->activeOrigin = activeOrigin;
	pActionData->bActive = bActive;
	pActionData->bChanged = bChanged;
	pActionData->bState = bState;
	pActionData->fUpdateTime = fUpdateTime;

	return VRInputError_None;
}
EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t *pActionData, uint32_t unActionDataSize,
	VRInputValueHandle_t ulRestrictToDevice) {

	float functionCallTimeInSeconds = BackendManager::Instance().GetTimeInSeconds();

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
		pActionData->fUpdateTime = 0;
		pActionData->deltaX = 0;
		pActionData->deltaY = 0;
		pActionData->deltaZ = 0;

		return VRInputError_None;
	}

	// determine input based on action path:
	VRInputValueHandle_t activeOrigin;
	VRControllerAxis_t *axisFromLastUpdate = nullptr;
	VRControllerAxis_t *axis = nullptr;
	string sourcePath;

	// ulRestrictToDevice may tell us input handle to look at if both inputs are available
	if (analogAction->leftInputValue != k_ulInvalidInputValueHandle &&
		analogAction->rightInputValue != k_ulInvalidInputValueHandle &&
		ulRestrictToDevice != vr::k_ulInvalidInputValueHandle)
	{
		activeOrigin = ulRestrictToDevice;
		InputValue *inputValue = (InputValue*)ulRestrictToDevice;

		axisFromLastUpdate = inputValue->controllerStateFromLastUpdate.rAxis;
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
		axisFromLastUpdate = inputValueLeft->controllerStateFromLastUpdate.rAxis;
		axis = inputValueLeft->controllerState.rAxis;
	}
	else if (analogAction->rightInputValue != k_ulInvalidInputValueHandle)
	{
		activeOrigin = analogAction->rightInputValue;
		sourcePath = analogAction->rightActionSources[0]->sourcePath;
		InputValue *inputValueRight = (InputValue*)analogAction->rightInputValue;
		axisFromLastUpdate = inputValueRight->controllerStateFromLastUpdate.rAxis;
		axis = inputValueRight->controllerState.rAxis;
	}


	// determine what data to output:
	InputValue *inputValue = (InputValue*)activeOrigin;
	string name = inputValue->name;
	string pathSubst = sourcePath.substr(name.size(), sourcePath.size() - name.size());
	VRControllerAxis_t analogDataFromLastUpdate;
	VRControllerAxis_t analogData;
	if (iequals(pathSubst, "/input/trackpad") || iequals(pathSubst, "/input/joystick"))
	{
		analogDataFromLastUpdate = axisFromLastUpdate[0];
		analogData = axis[0]; // thumbstick
	}
	else if (iequals(pathSubst, "/input/trigger"))
	{
		analogDataFromLastUpdate = axisFromLastUpdate[1];
		analogData = axis[1]; // trigger
	}
	else if (iequals(pathSubst, "/input/grip"))
	{
		analogDataFromLastUpdate = axisFromLastUpdate[2];
		analogData = axis[2]; // grip
	}

	float xDelta = inputValue->isSetControllerStateFromLastUpdate ? analogDataFromLastUpdate.x - analogData.x : 0;
	float yDelta = inputValue->isSetControllerStateFromLastUpdate ? analogDataFromLastUpdate.y - analogData.y : 0;

	float nowTimeInSeconds = BackendManager::Instance().GetTimeInSeconds();
	float fUpdateTime = functionCallTimeInSeconds - nowTimeInSeconds;

	pActionData->x = analogData.x;
	pActionData->y = analogData.y;
	pActionData->z = 0;  // todo: z is valid for vector3 actions
	pActionData->activeOrigin = activeOrigin;
	pActionData->bActive = true;
	pActionData->fUpdateTime = fUpdateTime;
	pActionData->deltaX = xDelta;
	pActionData->deltaY = yDelta;
	pActionData->deltaZ = 0; // todo: z is valid for vector3 actions

	return VRInputError_None;
}
EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
	InputPoseActionData_t *pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice) {

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
		if (fPredictedSecondsFromNow != 0)
		{
			ITrackedDevice *device = BackendManager::Instance().GetDevice(inputValue->trackedDeviceIndex);
			device->GetPose(eOrigin, &pActionData->pose, TrackingStateType_Prediction, fPredictedSecondsFromNow);
		}
		else
		{
			// Note that this forces dual-origin mode on the LibOVR driver (OculusDevice.cpp), but it seems
			// perfectly stable at this point.
			// Also note that to completely fix the input lag issue, passing TrackingStateType_Rendering into
			// GetPose had to happen in GetPoseActionData instead of UpdateActionState.

			ITrackedDevice *device = BackendManager::Instance().GetDevice(inputValue->trackedDeviceIndex);
			device->GetPose(eOrigin, &pActionData->pose, TrackingStateType_Rendering);
		}

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
	// Same function, different name - the 'RelativeToNow' suffix was added when GetPoseActionDataForNextFrame was added
	return GetPoseActionData(action, eOrigin, fPredictedSecondsFromNow, pActionData, unActionDataSize, ulRestrictToDevice);
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
EVRInputError BaseInput::GetDominantHand(vr::ETrackedControllerRole *peDominantHand) {
	STUBBED();
}
EVRInputError BaseInput::SetDominantHand(vr::ETrackedControllerRole eDominantHand) {
	STUBBED();
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

	InputValue *inputValue = nullptr;

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

	if (inputValue == nullptr) {
		OOVR_LOGF("WARN: Ignoring haptic action for unbound source: $d, %s", ulRestrictToDevice, vibrationAction->name.c_str());
		// Should this be InvalidDevice?
		return VRInputError_None;
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

	if (!digitalAction) {
		for (auto i = 0; i < originOutCount; i++) {
			originsOut[i] = {};
		}
		return VRInputError_InvalidHandle;
	}

	std::vector<VRInputValueHandle_t> vectorOriginOut;

	// Note: right now the action source is going to be either left or right controller...
	// In the future, they should be split up to controller parts (ex: button a)
	if (digitalAction->leftInputValue != k_ulInvalidInputValueHandle) {
		vectorOriginOut.push_back(digitalAction->leftInputValue);
	}

	if (digitalAction->rightInputValue != k_ulInvalidInputValueHandle) {
		vectorOriginOut.push_back(digitalAction->rightInputValue);
	}

	for(int i=0; i<originOutCount; i++) {
		if (i < vectorOriginOut.size()) {
			originsOut[i] = vectorOriginOut[i];
		} else {
			originsOut[i] = k_ulInvalidInputValueHandle;
		}
	}

	return VRInputError_None;
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize) {
	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char *pchNameArray, uint32_t unNameArraySize,
	int32_t unStringSectionsToInclude) {

	OOVR_FALSE_ABORT(origin != 0);

	InputValue* value = (InputValue*)origin;

	std::string tgt = "Localise::" + value->name;

	if (unNameArraySize < tgt.length())
		return VRInputError_BufferTooSmall;

	strcpy_s(pchNameArray, unNameArraySize, tgt.c_str());

	return VRInputError_None;
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

/** Retrieves useful information about the bindings for an action */
EVRInputError BaseInput::GetActionBindingInfo(VRActionHandle_t actionHandle, OOVR_InputBindingInfo_t *pOriginInfo,
		uint32_t unBindingInfoSize, uint32_t unBindingInfoCount, uint32_t *punReturnedBindingInfoCount ) {

	// TODO confirm this is correct

	auto *action = (Action *) actionHandle;

	// In OpenVR 1.10.30 the size of InputBindingInfo changed when they added rchInputSourceType.
	// Thus we have to work with both sizes, and can't just assert it to be the same as our struct.
	// To implement this we'll write into our full-size struct, then copy it out later.
	// This does mean a limit on the maximum number of bindings, but this should be more than enough.
	const int maxSourceCount = 32;
	OOVR_InputBindingInfo_t fullInfo[maxSourceCount] = {};

	// Zero out the input infos, while respecting whatever size they may have
	ZeroMemory(pOriginInfo, unBindingInfoSize * unBindingInfoCount);

	// For some reason No Man's Sky calls this with a nullptr handle
	// Haven't confirmed this is how SteamVR behaves, I'm guessing
	// it just returns InvalidHandle and does nothing, and that seems
	// to work.
	if (!action) {
		*punReturnedBindingInfoCount = 0;
		return VRInputError_InvalidHandle;
	}

	uint32_t i = 0;

	for (const ActionSource *src : action->leftActionSources) {
		// Note about fencepost errors: it's fine to exit the loop with i == maxSourceCount, just not add one more
		if (i >= maxSourceCount)
			goto tooManyActions;
		GetActionSourceBindingInfo(action, src, &fullInfo[i++]);
	}

	for (const ActionSource *src : action->rightActionSources) {
		if (i >= maxSourceCount)
			goto tooManyActions;
		GetActionSourceBindingInfo(action, src, &fullInfo[i++]);
	}

	// If we hit the limit, we should (tested in SteamVR) return VRInputError_BufferTooSmall, return the
	//  real number of actions in punReturnedBindingInfoCount, and leave pOriginInfo zeroed out.
	// Until <commit ID> (see #199) we didn't handle this properly and just returned as many as there was
	//  room in the buffer for, but now we do this properly.
	// TODO insert the commit IDs where we fixed this.
	*punReturnedBindingInfoCount = i;
	if (i > unBindingInfoCount) {
		return VRInputError_BufferTooSmall;
	}

	// Make sure we know how to handle this version of the struct
	// We assume here that the struct size always changes if a change is made, which seems perfectly reasonable.
	switch (unBindingInfoSize) {
	case sizeof(OOVR_InputBindingInfo_t):
		// If it's the newest version of the struct that's fine, just copy 1:1
		break;
	case 512:
		// This is how large the struct was prior to OpenVR 1.10.30 - since a field was added to
		//  the end of the struct, we're safe to just copy the first unBindingInfoSize bytes over.
		break;
	default:
		OOVR_ABORTF("BaseInput: InputBindingInfo_t is unknown length: %u", unBindingInfoSize);
	}

	// Copy everything into the output buffer
	for (int j = 0; j < i; j++) {
		// Since the size of InputBindingInfo_t might be different at runtime, we can't just index it normally
		// Thus calculate the offset in using the real size instead
		char* realOriginInfo = ((char*)pOriginInfo) + (j * unBindingInfoSize);
		memcpy(realOriginInfo, &fullInfo[j], unBindingInfoSize);
	}

	return VRInputError_None;

tooManyActions:
	// Not sure what the appropriate error here is, this is probably sufficiently obscure to match the
	//  obscurity of a default binding attaching 32 sources to one action.
	// Ideally we'd return upto the limit of what we can handle and skip the rest, but this code will most
	//  likely never run.
	*punReturnedBindingInfoCount = 0;
	return VRInputError_MaxCapacityReached;
}

void BaseInput::GetActionSourceBindingInfo(const Action *action,
		const ActionSource *src, OOVR_InputBindingInfo_t *result) {

	*result = {0};

	// Some sample values from the SteamVR OpenGL example:
	// {
	//   rchDevicePathName = "/user/hand/right"
	//   rchInputPathName = "/input/trigger"
	//   rchModeName = "button"
	//   rchSlotName = "click"
	//   rchInputSourceType = "trigger"
	// }

	// TODO cleanup with define
	strcpy_s(result->rchModeName, sizeof(result->rchModeName), src->sourceMode.c_str());
	strcpy_s(result->rchSlotName, sizeof(result->rchSlotName), src->sourceType.c_str());

	// Split the input pathname (eg "/user/hand/right/input/trigger") into a device
	// path ("/user/hand/right") and input path ("/input/trigger")
	strcpy_s(result->rchDevicePathName, sizeof(result->rchDevicePathName), src->sourceDevice.c_str());

	string inputPath = src->sourcePath.substr(src->sourceDevice.size());
	strcpy_s(result->rchInputPathName, sizeof(result->rchInputPathName), inputPath.c_str());

	strcpy_s(result->rchInputSourceType, sizeof(result->rchInputSourceType), src->sourcePhysicalType.c_str());

	// Note there still seems to be some issues with No Man's Sky's control display - it's showing everything
	// as belonging to the left controller. Not sure whether that's the fault of this code or not, but in
	// any case it should be noted in #126.
}

EVRInputError BaseInput::ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle) {
	STUBBED();
}
EVRInputError BaseInput::ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t *pSets, uint32_t unSizeOfVRSelectedActionSet_t,
	uint32_t unSetCount, VRInputValueHandle_t originToHighlight) {

	STUBBED();
}

EVRInputError BaseInput::GetComponentStateForBinding(const char *pchRenderModelName, const char *pchComponentName,
		const OOVR_InputBindingInfo_t *pOriginInfo, uint32_t unBindingInfoSize, uint32_t unBindingInfoCount,
		vr::RenderModel_ComponentState_t *pComponentState) {
	STUBBED();
}

bool BaseInput::IsUsingLegacyInput() {
	STUBBED();
}

// Interestingly enough this was added to IVRInput_007 without bumping the version number - that's fine since it's
// at the end of the vtable, but it's interesting that the version has always been bumped for this in the past.
EVRInputError BaseInput::OpenBindingUI(const char *pchAppKey, VRActionSetHandle_t ulActionSetHandle,
		VRInputValueHandle_t ulDeviceHandle, bool bShowOnDesktop) {
	STUBBED();
}

EVRInputError BaseInput::GetBindingVariant(vr::VRInputValueHandle_t ulDevicePath, char *pchVariantArray, uint32_t unVariantArraySize) {
	STUBBED();
}
