#include "stdafx.h"
#define BASE_IMPL
#include "BaseInput.h"
#include <string>

#include <convert.h>

#include "Drivers/Backend.h"
#include "static_bases.gen.h"
#include <algorithm>
#include <codecvt>
#include <fstream>
#include <locale>
#include <map>

#include "Misc/Input/InteractionProfile.h"

using namespace vr;

// This is a duplicate from BaseClientCore.cpp
static bool ReadJson(const std::wstring& path, Json::Value& result)
{
#ifndef _WIN32
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::ifstream in(converter.to_bytes(path), std::ios::binary);
#else
	std::ifstream in(path, std::ios::binary);
#endif
	if (in) {
		std::stringstream contents;
		contents << in.rdbuf();
		contents >> result;
		return true;
	} else {
		result = Json::Value(Json::ValueType::objectValue);
		return false;
	}
}

// Convert a UTF-8 string to a UTF-16 (wide) string
static std::wstring utf8to16(const std::string& t_str)
{
	//setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes(t_str);
}

static std::string dirnameOf(const std::string& fname)
{
	size_t pos = fname.find_last_of("\\/");
	return (std::string::npos == pos)
	    ? ""
	    : fname.substr(0, pos);
}

// Case-insensitively compares two strings
static bool iequals(const std::string& a, const std::string& b)
{
	// from https://stackoverflow.com/a/4119881
	size_t sz = a.size();
	if (b.size() != sz)
		return false;
	for (unsigned int i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}

// ASCII-only to-lower-case - for use with strings in maps, since steamvr is case-independent (In which
// locale you might wonder? Good question).
static std::string lowerStr(const std::string& in)
{
	std::string out;
	out.reserve(in.size());

	for (char c : in) {
		if (c >= 'A' && c <= 'Z') {
			c = (char)(c - 'A' + 'a');
		}
		out.push_back(c);
	}

	return out;
}

static void stringSplit(const std::string& str, std::vector<std::string>& items)
{
	items.clear();

	for (size_t i = 0; i < str.size();) {
		size_t nextStroke = str.find('/', i);

		// If the path starts with a stroke, ignore it - that's a very common case
		if (nextStroke == 0) {
			i++;
			continue;
		}

		// No more strokes in the string? This is the last bit
		if (nextStroke == std::string::npos) {
			items.push_back(str.substr(i));
			break;
		}

		items.push_back(str.substr(i, nextStroke - i));

		// Skip to one past the next stroke
		i = nextStroke + 1;
	}
}

// Converts an arbitrary application-supplied string to one we can use with OpenXR
static std::string escapePathString(const std::string& str)
{
	std::vector<char> out;
	out.reserve(str.length() + 10);
	for (char c : str) {
		// Valid list of characters, see the OpenXR spec part 6.2
		if (('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '/') {
			out.push_back(c);
			continue;
		}

		// Special-case uppercase letters by prefixing them with an underscore and lowering them
		// This will nicely convert CamelCase to snake_case, though THIS_CASE becomes ugly.
		if ('A' <= c && c <= 'Z') {
			out.push_back('_');
			out.push_back(c - 'A' + 'a');
			continue;
		}

		// Just print out the hex code
		char hex[3];
		snprintf(hex, sizeof(hex), "%02x", c);
		out.push_back('.');
		out.push_back(hex[0]);
		out.push_back(hex[1]);
	}

	return std::string(out.data(), out.size());
}

static std::string pathFromParts(const std::initializer_list<std::string>& parts)
{
	std::string str;
	for (const std::string& part : parts) {
		str += "/";
		str += part;
	}
	return str;
}

// ---

EVRInputError BaseInput::SetActionManifestPath(const char* pchActionManifestPath)
{
	OOVR_LOGF("Loading manifest file '%s'", pchActionManifestPath);

	//////////////
	//// Load the actions from the manifest file
	//////////////

	if (hasLoadedActions)
		OOVR_ABORT("Cannot re-load actions!");
	hasLoadedActions = true;

	Json::Value root;
	// It says 'open or parse', but really it ignores parse errors - TODO catch those
	if (!ReadJson(utf8to16(pchActionManifestPath), root))
		OOVR_ABORT("Failed to open or parse input manifest");

	// Parse the actions
	for (Json::Value item : root["actions"]) {
		Action action = {};
		action.fullName = lowerStr(item["name"].asString());

		// Split the full name by stroke ('/') characters
		// They have four parts: the first is always 'actions', the second is the action set name, the third is
		// either 'in' or 'out' (in for inputs, out for haptics) and the last is the action's short name.
		std::vector<std::string> parts;
		stringSplit(action.fullName, parts);

		if (parts.size() != 4)
			OOVR_ABORTF("Invalid action name '%s' - wrong number of parts %d", action.fullName.c_str(), parts.size());

		if (parts.at(0) != "actions")
			OOVR_ABORTF("Invalid action name '%s' - bad first parts '%s'", action.fullName.c_str(), parts.at(0).c_str());

		action.setName = parts.at(1);
		action.shortName = parts.at(3);

		std::string inOut = parts.at(2);
		if (inOut == "in")
			action.haptic = false;
		else if (inOut == "out")
			action.haptic = true;
		else
			OOVR_ABORTF("Invalid action name '%s' - bad in/out value '%s'", action.fullName.c_str(), inOut.c_str());

		// Parse the requirement
		// TODO default to optional
		std::string requirement = item["requirement"].asString();
		if (requirement == "mandatory")
			action.requirement = ActionRequirement::Mandatory;
		else if (requirement == "suggested" || requirement.empty()) // Default
			action.requirement = ActionRequirement::Suggested;
		else if (requirement == "optional")
			action.requirement = ActionRequirement::Optional;
		else
			OOVR_ABORTF("Invalid action requirement value '%s' for action '%s'", requirement.c_str(), action.fullName.c_str());

		// Parse the type
		std::string type = item["type"].asString();
		if (type == "boolean")
			action.type = ActionType::Boolean;
		else if (type == "vector1")
			action.type = ActionType::Vector1;
		else if (type == "vector2")
			action.type = ActionType::Vector2;
		else if (type == "vector3")
			action.type = ActionType::Vector3;
		else if (type == "vibration")
			action.type = ActionType::Vibration;
		else if (type == "pose")
			action.type = ActionType::Pose;
		else if (type == "skeleton")
			action.type = ActionType::Skeleton;
		else
			OOVR_ABORTF("Invalid action type '%s' for action '%s'", type.c_str(), action.fullName.c_str());

		if (actions.count(action.fullName))
			OOVR_ABORTF("Duplicate action name '%s'", action.fullName.c_str());

		actions[action.fullName] = std::make_unique<Action>(action);
	}

	// Parse the action sets
	for (Json::Value item : root["action_sets"]) {
		ActionSet set = {};

		set.fullName = lowerStr(item["name"].asString());

		// Split the full name by stroke ('/') characters
		// They have four parts: the first is always 'actions', the second is the action set name, the third is
		// either 'in' or 'out' (in for inputs, out for haptics) and the last is the action's short name.
		std::vector<std::string> parts;
		stringSplit(set.fullName, parts);

		if (parts.size() != 2)
			OOVR_ABORTF("Invalid action set name '%s' - wrong number of parts %d", set.fullName.c_str(), parts.size());

		if (parts.at(0) != "actions")
			OOVR_ABORTF("Invalid action name '%s' - bad first parts '%s'", set.fullName.c_str(), parts.at(0).c_str());

		set.name = parts.at(1);

		// Find the usage
		std::string usage = item["usage"].asString();
		if (usage == "leftright")
			set.usage = ActionSetUsage::LeftRight;
		else if (usage == "single")
			set.usage = ActionSetUsage::Single;
		else if (usage == "hidden")
			set.usage = ActionSetUsage::Hidden;
		else
			OOVR_ABORTF("Invalid action set usage '%s' for action set '%s'", usage.c_str(), set.name.c_str());

		// Register it
		actionSets[set.name] = std::make_unique<ActionSet>(set);
	}

	// Make sure all the actions have a corresponding set
	for (auto& pair : actions) {
		Action& action = *pair.second;

		// TODO in the OpenVR samples, they don't declare their action set. Are they automatically created?!
		auto item = actionSets.find(action.setName);
		if (item == actionSets.end())
			OOVR_ABORTF("Invalid action set '%s' for action '%s'", action.setName.c_str(), action.fullName.c_str());

		action.set = item->second.get();
	}

	// Find the default bindings file
	// TODO load all of them, and let the OpenXR runtime choose which one to use
	std::string bestPath;
	int bestPriority = -1;
	for (Json::Value item : root["default_bindings"]) {
		std::string type = item["type"].asString();

		// Given the type of controller, find a priority for it
		int priority;

		if (iequals(type, "oculus_touch"))
			priority = 3;
		else if (iequals(type, "rift")) // This came from the previous code, where's it from?
			priority = 2;
		else if (iequals(type, "generic"))
			priority = 1;
		else
			priority = 0;

		if (priority > bestPriority)
			bestPath = item["binding_url"].asString();
	}

	if (bestPath.empty())
		OOVR_ABORT("No compatible binding action specified!");

	bindingsPath = dirnameOf(pchActionManifestPath) + "/" + bestPath;

	//////////////////////////
	/// Now we've got everything done, load the actions into OpenXR
	//////////////////////////

	for (auto& pair : actionSets) {
		ActionSet& as = *pair.second;

		XrActionSetCreateInfo createInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
		std::string safeName = escapePathString(as.name);
		strcpy_arr(createInfo.actionSetName, safeName.c_str());
		strcpy_arr(createInfo.localizedActionSetName, as.name.c_str()); // TODO localisation

		// Take priority over the ActionSet which defines the legacy bindings.
		createInfo.priority = 100;

		OOVR_FAILED_XR_ABORT(xrCreateActionSet(xr_instance, &createInfo, &as.xr));
	}

	for (auto& pair : actions) {
		Action& act = *pair.second;

		XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };
		std::string safeName = escapePathString(act.shortName);
		strcpy_arr(info.actionName, safeName.c_str());
		strcpy_arr(info.localizedActionName, act.shortName.c_str()); // TODO localisation

		switch (act.type) {
		case ActionType::Boolean:
			info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
			break;
		case ActionType::Vector1:
			info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
			break;
		case ActionType::Vector2:
			info.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
			break;
		case ActionType::Vector3:
			STUBBED(); // Not XR_STUBBED since this didn't work before, and I've certainly never heard of a use for it
			break;
		case ActionType::Vibration:
			info.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
			break;
		case ActionType::Pose:
			info.actionType = XR_ACTION_TYPE_POSE_INPUT;
			break;
		case ActionType::Skeleton:
			STUBBED(); // Not XR_STUBBED since AFAIK this didn't work before, and since OpenXR doesn't do skeletal stuff we'll have to sort this our ourselves
			break;
		default:
			OOVR_ABORTF("Bad action type while remapping action %s: %d", act.fullName.c_str(), act.type);
		}

		OOVR_FAILED_XR_ABORT(xrCreateAction(act.set->xr, &info, &act.xr));
	}

	// Read the default bindings file, and load it into OpenXR
	std::vector<XrActionSuggestedBinding> bindings;
	OculusTouchInteractionProfile profile;
	LoadBindingsSet(bindingsPath, profile, bindings);

	// Add our legacy bindings in - these are what power GetControllerState
	AddLegacyBindings(profile, bindings);

	// Load the bindings into the runtime
	XrPath interactionProfilePath;
	OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, profile.GetPath().c_str(), &interactionProfilePath));
	XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	suggestedBindings.interactionProfile = interactionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = bindings.size();
	OOVR_FAILED_XR_ABORT(xrSuggestInteractionProfileBindings(xr_instance, &suggestedBindings));

	// Attach everything to the current session
	BindInputsForSession();

	return vr::VRInputError_None;
}

void BaseInput::LoadEmptyManifest()
{
	OOVR_LOG("Loading virtual empty manifest");

	if (hasLoadedActions)
		OOVR_ABORT("Cannot re-load actions!");
	hasLoadedActions = true;
	usingLegacyInput = true;

	// TODO deduplicate with the regular manifest loader
	std::vector<XrActionSuggestedBinding> bindings;
	OculusTouchInteractionProfile profile;
	AddLegacyBindings(profile, bindings);

	// Load the bindings into the runtime
	XrPath interactionProfilePath;
	OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, profile.GetPath().c_str(), &interactionProfilePath));
	XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	suggestedBindings.interactionProfile = interactionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = bindings.size();
	OOVR_FAILED_XR_ABORT(xrSuggestInteractionProfileBindings(xr_instance, &suggestedBindings));

	// Attach everything to the current session
	BindInputsForSession();
}

void BaseInput::BindInputsForSession()
{
	// Note: even if actionSets is empty, we always still want to load the legacy set.

	// Now attach the action sets to the OpenXR session, making them immutable (including attaching suggested bindings)
	std::vector<XrActionSet> sets;
	for (auto& pair : actionSets) {
		ActionSet& as = *pair.second;
		sets.push_back(as.xr);
	}

	sets.push_back(legacyInputsSet);

	XrSessionActionSetsAttachInfo attachInfo = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attachInfo.actionSets = sets.data();
	attachInfo.countActionSets = sets.size();
	OOVR_FAILED_XR_ABORT(xrAttachSessionActionSets(xr_session, &attachInfo));
}

void BaseInput::LoadBindingsSet(const std::string& bindingsPath, const struct InteractionProfile& profile, std::vector<XrActionSuggestedBinding>& bindings)
{
	Json::Value bindingsRoot;
	if (!ReadJson(utf8to16(bindingsPath), bindingsRoot)) {
		OOVR_ABORTF("Failed to read and parse JSON binding descriptor: %s", bindingsPath.c_str());
	}

	// TODO aliases, if anyone uses them
	if (!bindingsRoot["alias_info"].empty())
		OOVR_LOGF("WARNING: Ignoring alias_info from binding descriptor %s", bindingsPath.c_str());

	const Json::Value& bindingsJson = bindingsRoot["bindings"];
	if (!bindingsJson.isObject())
		OOVR_ABORTF("Invalid bindings file %s, missing or invalid bindings object", bindingsPath.c_str());

	for (std::string setFullName : bindingsJson.getMemberNames()) {
		const Json::Value setJson = bindingsJson[setFullName];

		setFullName = lowerStr(setFullName);

		std::string prefix = "/actions/";
		if (strncmp(prefix.c_str(), setFullName.c_str(), prefix.size()) != 0) {
			OOVR_ABORTF("Invalid action set name '%s' in bindings file '%s' - missing or bad prefix", setFullName.c_str(), bindingsPath.c_str());
		}
		std::string setName = setFullName.substr(prefix.size());

		auto setIter = actionSets.find(setName);
		if (setIter == actionSets.end())
			OOVR_ABORTF("Missing action set '%s' in bindings file '%s'", setName.c_str(), bindingsPath.c_str());
		const ActionSet& set = *setIter->second;

		for (const auto& srcJson : setJson["sources"]) {
			std::string importBasePath = lowerStr(srcJson["path"].asString());

			const Json::Value& inputsJson = srcJson["inputs"];
			for (const std::string& inputName : inputsJson.getMemberNames()) {
				const Json::Value item = inputsJson[inputName];

				std::string actionName = lowerStr(item["output"].asString());
				auto actionIter = actions.find(actionName);
				if (actionIter == actions.end())
					OOVR_ABORTF("Missing action '%s' in bindings file '%s'", actionName.c_str(), bindingsPath.c_str());
				const Action& action = *actionIter->second;

				// There's probably some differences, but it looks like the SteamVR paths will 'just work' with OpenXR
				// FIXME this doesn't with with binding boolean actions to analogue inputs
				std::string pathStr = importBasePath + "/" + inputName;

				if (!profile.IsInputPathValid(pathStr)) {
					OOVR_LOGF("WARNING: Built invalid input path %s for profile %s from action %s, skipping",
					    pathStr.c_str(), profile.GetPath().c_str(), actionName.c_str());
					continue;
				}

				XrPath path;
				OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, pathStr.c_str(), &path));
				bindings.push_back(XrActionSuggestedBinding{ action.xr, path });
			}
		}
	}

	// If there aren't any bindings, that makes it simple
	// If we didn't return then xrSuggestInteractionProfileBindings would fail
	if (bindings.empty()) {
		return;
	}

	// Load the bindings we just built into the runtime
	XrPath interactionProfilePath;
	OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, profile.GetPath().c_str(), &interactionProfilePath));

	XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggestedBindings.interactionProfile = interactionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = bindings.size();
	OOVR_FAILED_XR_ABORT(xrSuggestInteractionProfileBindings(xr_instance, &suggestedBindings));
}

void BaseInput::AddLegacyBindings(InteractionProfile& profile, std::vector<XrActionSuggestedBinding>& bindings)
{
	// Add the stuff required to make the backend work
	// This means the pose and haptic inputs for each hand, and actions for all the legacy inputs, into a new ActionSet that
	// is always active with a lower priority than the game's one.

	XrActionSetCreateInfo setInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy_arr(setInfo.actionSetName, "opencomposite-actions");
	strcpy_arr(setInfo.localizedActionSetName, "OpenComposite Actions");
	setInfo.priority = 0;

	OOVR_FAILED_XR_ABORT(xrCreateActionSet(xr_instance, &setInfo, &legacyInputsSet));

	for (int i = 0; i < 2; i++) {
		LegacyControllerActions& ctrl = legacyControllers[i];
		ctrl = {};

		std::string side = i == 0 ? "left" : "right";
		std::string prefix = "/user/hand/" + side + "/";

		auto createSpecial = [&](XrAction* out, const std::string& path, const std::string& name, const std::string& humanName, XrActionType type) {
			XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };
			info.actionType = type;

			std::string codeName = "legacy-" + side + "-" + name;
			strcpy_arr(info.actionName, codeName.c_str());

			std::string fullHumanName = "Legacy input: " + humanName + " - " + side;
			strcpy_arr(info.localizedActionName, fullHumanName.c_str());

			OOVR_FAILED_XR_ABORT(xrCreateAction(legacyInputsSet, &info, out));

			// Bind the action to a suggested binding
			std::string realPath = prefix + path;
			if (!profile.IsInputPathValid(realPath)) {
				OOVR_LOGF("Skipping legacy input binding %s, not supported by profile", realPath.c_str());
				return;
			}

			XrActionSuggestedBinding binding = {};
			binding.action = *out;

			OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, realPath.c_str(), &binding.binding));

			bindings.push_back(binding);
		};

		auto create = [&](XrAction* out, const std::string& path, const std::string& name, const std::string& humanName, XrActionType type) {
			createSpecial(out, "input/" + path, name, humanName, type);
		};

#define BOOL_WITH_CAP(member, path, name, humanName)                                                             \
	do {                                                                                                         \
		create(member, path "/click", name, humanName, XR_ACTION_TYPE_BOOLEAN_INPUT);                            \
		create(member##Touch, path "/touch", name "-touch", humanName " (touch)", XR_ACTION_TYPE_BOOLEAN_INPUT); \
	} while (0)

		if (i == 0) {
			// Left
			BOOL_WITH_CAP(&ctrl.menu, "y", "y", "Upper-left (Y)");
			BOOL_WITH_CAP(&ctrl.btnA, "x", "x", "Lower-left (X)");

			// Note this refers to what Oculus calls the menu button (and games use to open the pause menu), which
			// is used by SteamVR for it's menu.
			create(&ctrl.system, "menu/click", "menu", "Menu", XR_ACTION_TYPE_BOOLEAN_INPUT);
		} else {
			// Right
			BOOL_WITH_CAP(&ctrl.menu, "b", "b", "Upper-right (B)");
			BOOL_WITH_CAP(&ctrl.btnA, "a", "a", "Lower-right (A)");

			// Ignore Oculus's system button
		}

		// Thumbstick button
		BOOL_WITH_CAP(&ctrl.stickBtn, "thumbstick", "stick", "Thumbstick");

#undef BOOL_WITH_CAP

		create(&ctrl.grip, "squeeze/value", "grip", "Grip", XR_ACTION_TYPE_FLOAT_INPUT);

		create(&ctrl.trigger, "trigger/value", "trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT);
		create(&ctrl.triggerTouch, "trigger/touch", "trigger-touch", "Trigger (touch)", XR_ACTION_TYPE_BOOLEAN_INPUT);

		create(&ctrl.stickX, "thumbstick/x", "thumbstick-x", "Thumbstick X axis", XR_ACTION_TYPE_FLOAT_INPUT);
		create(&ctrl.stickY, "thumbstick/y", "thumbstick-y", "Thumbstick Y axis", XR_ACTION_TYPE_FLOAT_INPUT);

		createSpecial(&ctrl.haptic, "output/haptic", "haptic", "Haptics", XR_ACTION_TYPE_VIBRATION_OUTPUT);

		create(&ctrl.gripPoseAction, "grip/pose", "grip-pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT);
		create(&ctrl.aimPoseAction, "aim/pose", "aim-pose", "Aim Pose", XR_ACTION_TYPE_POSE_INPUT);
	}
}

EVRInputError BaseInput::GetActionSetHandle(const char* pchActionSetName, VRActionSetHandle_t* pHandle)
{
	*pHandle = k_ulInvalidActionSetHandle;

	std::string prefix = "/actions/";
	if (strncmp(prefix.c_str(), pchActionSetName, prefix.size()) != 0) {
		OOVR_ABORTF("Invalid action set name '%s' - missing or bad prefix", pchActionSetName);
	}
	std::string setName = pchActionSetName + prefix.size();

	auto item = actionSets.find(lowerStr(setName));
	if (item == actionSets.end())
		return VRInputError_NameNotFound;

	*pHandle = (VRActionSetHandle_t)item->second.get();
	return VRInputError_None;
}

EVRInputError BaseInput::GetActionHandle(const char* pchActionName, VRActionHandle_t* pHandle)
{
	*pHandle = k_ulInvalidActionHandle;

	auto item = actions.find(lowerStr(pchActionName));
	if (item == actions.end())
		return VRInputError_NameNotFound;

	*pHandle = (VRActionHandle_t)item->second.get();
	return VRInputError_None;
}

EVRInputError BaseInput::GetInputSourceHandle(const char* pchInputSourcePath, VRInputValueHandle_t* pHandle)
{
	STUBBED();
}

EVRInputError BaseInput::UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets,
    uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount)
{
	// TODO if the game is using legacy input, call this every frame

	OOVR_FALSE_ABORT(sizeof(*pSets) == unSizeOfVRSelectedActionSet_t);

	// TODO support multiple action sets at once
	OOVR_FALSE_ABORT(unSetCount == 1);

	// TODO use all the other VRActiveActionSet properties
	ActionSet* as = cast_ASH(pSets->ulActionSet);

	const int as_count = 2;
	XrActiveActionSet aas[as_count];
	ZeroMemory(aas, sizeof(aas));

	aas[0].actionSet = as->xr;
	aas[1].actionSet = legacyInputsSet;

	XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
	syncInfo.activeActionSets = aas;
	syncInfo.countActiveActionSets = as_count;
	OOVR_FAILED_XR_ABORT(xrSyncActions(xr_session, &syncInfo));

	return VRInputError_None;
}

void BaseInput::InternalUpdate()
{
	if (!usingLegacyInput)
		return;

	XrActiveActionSet aas = { legacyInputsSet };

	XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
	syncInfo.activeActionSets = &aas;
	syncInfo.countActiveActionSets = 1;
	OOVR_FAILED_XR_ABORT(xrSyncActions(xr_session, &syncInfo));
}

EVRInputError BaseInput::GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	Action* act = cast_AH(action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	// TODO implement ulRestrictToDevice
	OOVR_FALSE_ABORT(ulRestrictToDevice == vr::k_ulInvalidInputValueHandle);

	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = act->xr;

	XrActionStateBoolean state = { XR_TYPE_ACTION_STATE_BOOLEAN };

	OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session, &getInfo, &state));

	pActionData->bState = state.currentState;
	pActionData->bActive = state.isActive;
	pActionData->bChanged = state.changedSinceLastSync;
	// TODO implement fUpdateTime
	// TODO implement activeOrigin

	return VRInputError_None;
}

EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
    InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}

EVRInputError BaseInput::GetPoseActionDataRelativeToNow(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	// Same function, different name - the 'RelativeToNow' suffix was added when GetPoseActionDataForNextFrame was added
	return GetPoseActionData(action, eOrigin, fPredictedSecondsFromNow, pActionData, unActionDataSize, ulRestrictToDevice);
}
EVRInputError BaseInput::GetPoseActionDataForNextFrame(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	return GetPoseActionData(action, eOrigin, 0, pActionData, unActionDataSize, ulRestrictToDevice);
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t action, InputSkeletalActionData_t* pActionData, uint32_t unActionDataSize)
{
	STUBBED();
}
EVRInputError BaseInput::GetDominantHand(vr::ETrackedControllerRole* peDominantHand)
{
	STUBBED();
}
EVRInputError BaseInput::SetDominantHand(vr::ETrackedControllerRole eDominantHand)
{
	STUBBED();
}
EVRInputError BaseInput::GetBoneCount(VRActionHandle_t action, uint32_t* pBoneCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetBoneHierarchy(VRActionHandle_t action, VR_ARRAY_COUNT(unIndexArayCount) BoneIndex_t* pParentIndices, uint32_t unIndexArayCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetBoneName(VRActionHandle_t action, BoneIndex_t nBoneIndex, VR_OUT_STRING() char* pchBoneName, uint32_t unNameBufferSize)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalReferenceTransforms(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace, EVRSkeletalReferencePose eReferencePose, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalTrackingLevel(VRActionHandle_t action, EVRSkeletalTrackingLevel* pSkeletalTrackingLevel)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray,
    uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t action, EVRSummaryType eSummaryType, VRSkeletalSummaryData_t* pSkeletalSummaryData)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t action, VRSkeletalSummaryData_t* pSkeletalSummaryData)
{
	return GetSkeletalSummaryData(action, VRSummaryType_FromDevice, pSkeletalSummaryData);
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_OUT_BUFFER_COUNT(unCompressedSize) void* pvCompressedData, uint32_t unCompressedSize,
    uint32_t* punRequiredCompressedSize, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}
EVRInputError BaseInput::GetSkeletalBoneDataCompressed(VRActionHandle_t action, EVRSkeletalMotionRange eMotionRange,
    VR_OUT_BUFFER_COUNT(unCompressedSize) void* pvCompressedData, uint32_t unCompressedSize, uint32_t* punRequiredCompressedSize)
{
	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(void* pvCompressedBuffer, uint32_t unCompressedBufferSize,
    EVRSkeletalTransformSpace* peTransformSpace, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray,
    uint32_t unTransformArrayCount)
{

	STUBBED();
}
EVRInputError BaseInput::DecompressSkeletalBoneData(const void* pvCompressedBuffer, uint32_t unCompressedBufferSize, EVRSkeletalTransformSpace eTransformSpace,
    VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	STUBBED();
}

EVRInputError BaseInput::TriggerHapticVibrationAction(VRActionHandle_t action, float fStartSecondsFromNow, float fDurationSeconds,
    float fFrequency, float fAmplitude, VRInputValueHandle_t ulRestrictToDevice)
{
	STUBBED();
}

EVRInputError BaseInput::GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle,
    VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t* originsOut, uint32_t originOutCount)
{
	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize)
{
	STUBBED();
}
EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize,
    int32_t unStringSectionsToInclude)
{

	STUBBED();
}
EVRInputError BaseInput::GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t* pOriginInfo, uint32_t unOriginInfoSize)
{
	STUBBED();
}

/** Retrieves useful information about the bindings for an action */
EVRInputError BaseInput::GetActionBindingInfo(VRActionHandle_t actionHandle, OOVR_InputBindingInfo_t* bindingInfo,
    uint32_t unBindingInfoSize, uint32_t unBindingInfoCount, uint32_t* punReturnedBindingInfoCount)
{
	memset(bindingInfo, 0, unBindingInfoSize * unBindingInfoCount);
	if (punReturnedBindingInfoCount)
		*punReturnedBindingInfoCount = 0;

	OOVR_FALSE_ABORT(unBindingInfoSize == sizeof(OOVR_InputBindingInfo_t));

	// FIXME support any number of sources
	// TODO does this support passing in unBindingInfoSize=0 and reading the required size? Check with SteamVR.
	const Action* action = cast_AH(actionHandle);

	XrBoundSourcesForActionEnumerateInfo enumInfo = { XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO };
	enumInfo.action = action->xr;
	uint32_t sourcesCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session, &enumInfo, 0, &sourcesCount, nullptr));
	std::vector<XrPath> boundActionPaths(sourcesCount);
	OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session, &enumInfo, boundActionPaths.size(), &sourcesCount, boundActionPaths.data()));

	// TODO should we return an error if there are no sources bound?

	int count = boundActionPaths.size();
	if (count > unBindingInfoCount)
		count = unBindingInfoCount;
	if (punReturnedBindingInfoCount)
		*punReturnedBindingInfoCount = count;

	for (int i = 0; i < count; i++) {
		OOVR_InputBindingInfo_t& info = bindingInfo[i];
		XrPath path = boundActionPaths.at(i);

		uint32_t pathLen;
		OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, path, 0, &pathLen, nullptr));
		std::vector<char> chars(pathLen);
		OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, path, chars.size(), &pathLen, chars.data()));
		std::string pathStr(chars.data(), chars.size());

		std::vector<std::string> parts;
		stringSplit(pathStr, parts);

		// The last part of the string - which is something like '/user/hand/right/input/a/click' - is the mode
		strcpy_arr(info.rchModeName, parts.back().c_str());

		// Hardcode the first three parts of the string as being the device path, and the 4th and 5th ones as
		// being the input path.
		std::string devicePath = pathFromParts({ parts.at(0), parts.at(1), parts.at(2) });
		std::string inputPath = pathFromParts({ parts.at(3), parts.at(4) });
		strcpy_arr(info.rchDevicePathName, devicePath.c_str());
		strcpy_arr(info.rchInputPathName, inputPath.c_str());

		// FIXME replace this initial hacky thing
		switch (action->type) {
		case ActionType::Boolean:
			strcpy_arr(bindingInfo->rchModeName, "button");
			strcpy_arr(bindingInfo->rchInputSourceType, "button");
			break;
		case ActionType::Vector1:
			strcpy_arr(bindingInfo->rchModeName, "trigger");
			strcpy_arr(bindingInfo->rchInputSourceType, "trigger");
			break;
		case ActionType::Vector2:
			strcpy_arr(bindingInfo->rchModeName, "joystick");
			strcpy_arr(bindingInfo->rchInputSourceType, "joystick");
			break;
		default:
			OOVR_ABORTF("Unimplemented action type %d for %s", action->type, pathStr.c_str());
		}
	}

	return VRInputError_None;
}

EVRInputError BaseInput::ShowActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t ulActionHandle)
{
	STUBBED();
}
EVRInputError BaseInput::ShowBindingsForActionSet(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets, uint32_t unSizeOfVRSelectedActionSet_t,
    uint32_t unSetCount, VRInputValueHandle_t originToHighlight)
{

	STUBBED();
}

EVRInputError BaseInput::GetComponentStateForBinding(const char* pchRenderModelName, const char* pchComponentName,
    const OOVR_InputBindingInfo_t* pOriginInfo, uint32_t unBindingInfoSize, uint32_t unBindingInfoCount,
    vr::RenderModel_ComponentState_t* pComponentState)
{
	STUBBED();
}

bool BaseInput::IsUsingLegacyInput()
{
	return usingLegacyInput;
}

// Interestingly enough this was added to IVRInput_007 without bumping the version number - that's fine since it's
// at the end of the vtable, but it's interesting that the version has always been bumped for this in the past.
EVRInputError BaseInput::OpenBindingUI(const char* pchAppKey, VRActionSetHandle_t ulActionSetHandle,
    VRInputValueHandle_t ulDeviceHandle, bool bShowOnDesktop)
{
	STUBBED();
}

EVRInputError BaseInput::GetBindingVariant(vr::VRInputValueHandle_t ulDevicePath, char* pchVariantArray, uint32_t unVariantArraySize)
{
	STUBBED();
}

BaseInput::Action* BaseInput::cast_AH(VRActionHandle_t handle)
{
	return (Action*)handle;
}

BaseInput::ActionSet* BaseInput::cast_ASH(VRActionSetHandle_t handle)
{
	return (ActionSet*)handle;
}

bool BaseInput::GetLegacyControllerState(vr::TrackedDeviceIndex_t controllerDeviceIndex, vr::VRControllerState_t* state)
{
	*state = {};

	// FIXME implement packetNum
	static int i = 0;
	state->unPacketNum = i++; // Not exactly thread safe

	// TODO for performance reasons, is it worth grabbing the results once and reusing them until xrSyncActions is called?

	int hand = DeviceIndexToHandId(controllerDeviceIndex);
	if (hand == -1)
		return false;
	LegacyControllerActions& ctrl = legacyControllers[hand];

	auto bindButton = [state](XrAction action, XrAction touch, int shift) {
		XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
		XrActionStateBoolean xs = { XR_TYPE_ACTION_STATE_BOOLEAN };

		if (action) {
			getInfo.action = action;
			OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session, &getInfo, &xs));
			state->ulButtonPressed |= (uint64_t)(xs.currentState != 0) << shift;
		}

		if (touch != XR_NULL_HANDLE) {
			getInfo.action = touch;
			OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session, &getInfo, &xs));
			state->ulButtonTouched |= (uint64_t)(xs.currentState != 0) << shift;
		}
	};

	// Read the buttons

	bindButton(ctrl.system, XR_NULL_HANDLE, vr::k_EButton_System);
	bindButton(ctrl.btnA, ctrl.btnATouch, vr::k_EButton_A);
	bindButton(ctrl.menu, ctrl.menuTouch, vr::k_EButton_ApplicationMenu);
	bindButton(ctrl.stickBtn, ctrl.stickBtnTouch, vr::k_EButton_SteamVR_Touchpad);

	// FIXME these two need to convert from an analogue value
	OOVR_LOG_ONCE("Analogue-to-digital conversion for trigger/grip not yet implemented");
#ifndef XR_STUBBED
#error todo
#endif
	bindButton(XR_NULL_HANDLE, ctrl.triggerTouch, vr::k_EButton_SteamVR_Trigger);
	bindButton(XR_NULL_HANDLE, XR_NULL_HANDLE, vr::k_EButton_Axis2);

	// Read the analogue values
	auto readFloat = [](XrAction action) -> float {
		if (!action)
			return 0;

		XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
		getInfo.action = action;

		XrActionStateFloat as = { XR_TYPE_ACTION_STATE_FLOAT };
		OOVR_FAILED_XR_ABORT(xrGetActionStateFloat(xr_session, &getInfo, &as));
		if (as.isActive) {
			return as.currentState;
		} else {
			return 0;
		}
	};

	VRControllerAxis_t& thumbstick = state->rAxis[0];
	thumbstick.x = readFloat(ctrl.stickX);
	thumbstick.y = readFloat(ctrl.stickY);

	VRControllerAxis_t& trigger = state->rAxis[1];
	trigger.x = readFloat(ctrl.trigger);
	trigger.y = 0;

	VRControllerAxis_t& grip = state->rAxis[2];
	grip.x = readFloat(ctrl.grip);
	grip.y = 0;

	// TODO implement the DPad actions
	OOVR_LOG_ONCE("DPad emulation not yet implemented");
#ifndef XR_STUBBED
#error todo
#endif

	return true;
}

void BaseInput::TriggerLegacyHapticPulse(vr::TrackedDeviceIndex_t controllerDeviceIndex, uint64_t durationNanos)
{
	int hand = DeviceIndexToHandId(controllerDeviceIndex);
	if (hand == -1)
		return;
	LegacyControllerActions& ctrl = legacyControllers[hand];

	if (!ctrl.haptic) {
		OOVR_LOG_ONCE("Cannot trigger haptic pulse, no haptic action present");
		return;
	}

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action = ctrl.haptic;

	XrHapticVibration vibration = { XR_TYPE_HAPTIC_VIBRATION };
	vibration.frequency = XR_FREQUENCY_UNSPECIFIED;
	vibration.duration = durationNanos;
	vibration.amplitude = 1;

	OOVR_FAILED_XR_ABORT(xrApplyHapticFeedback(xr_session, &info, (XrHapticBaseHeader*)&vibration));
}

int BaseInput::DeviceIndexToHandId(vr::TrackedDeviceIndex_t idx)
{
	ITrackedDevice* dev = BackendManager::Instance().GetDevice(idx);
	if (!dev)
		return false;

	ITrackedDevice::HandType hand = dev->GetHand();

	switch (hand) {
	case ITrackedDevice::HAND_LEFT:
		return 0;
	case ITrackedDevice::HAND_RIGHT:
		return 1;
	default:
		return -1;
	}
}

void BaseInput::GetHandSpace(vr::TrackedDeviceIndex_t index, XrSpace& space)
{
	space = XR_NULL_HANDLE;

	// If the manifest isn't loaded yet (still on the first frame) return null
	if (!hasLoadedActions)
		return;

	ITrackedDevice* dev = BackendManager::Instance().GetDevice(index);
	if (!dev)
		return;

	ITrackedDevice::HandType hand = dev->GetHand();
	LegacyControllerActions& ctrl = legacyControllers[hand];

	// Refs here so we can easily fiddle with them
	XrAction action = ctrl.aimPoseAction;
	XrSpace& actionSpace = ctrl.aimPoseSpace;

	// Create the action space if necessary
	if (!actionSpace) {
		XrActionSpaceCreateInfo info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		info.poseInActionSpace = S2O_om34_pose(G2S_m34(glm::identity<glm::mat4>()));
		info.action = action;
		OOVR_FAILED_XR_ABORT(xrCreateActionSpace(xr_session, &info, &actionSpace));
	}

	space = actionSpace;
}
