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
#include <utility>

#include "Misc/Input/KhrSimpleInteractionProfile.h"
#include "Misc/Input/OculusInteractionProfile.h"
#include "Misc/xrmoreutils.h"

using namespace vr;

// On Android, the application must supply a function to load the contents of a file
#include "Misc/android_api.h"

// This is a duplicate from BaseClientCore.cpp
static bool ReadJson(const std::wstring& path, Json::Value& result)
{
#ifndef _WIN32
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	const std::string real_path = converter.to_bytes(path);
#else
	const std::wstring& real_path = path;
#endif

#ifndef ANDROID
	std::ifstream in(real_path, std::ios::binary);
	if (in) {
		std::stringstream contents;
		contents << in.rdbuf();
		contents >> result;
		return true;
	} else {
		result = Json::Value(Json::ValueType::objectValue);
		return false;
	}
#else
	std::string contents = OpenComposite_Android_Load_Input_File(real_path.c_str());
	Json::Reader reader;
	reader.parse(contents, result, false);
	return true;
#endif
}

// Convert a UTF-8 string to a UTF-16 (wide) string
static std::wstring utf8to16(const std::string& t_str)
{
	// setup converter
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	// use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
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

// Must be defined non-inline to avoid it ending up in stubs.gen.cpp
BaseInput::LegacyControllerActions::~LegacyControllerActions() = default;
BaseInput::Action::~Action() = default;
BaseInput::ActionSource::~ActionSource() = default;

// ---

BaseInput::BaseInput()
{
	interactionProfiles.emplace_back(std::unique_ptr<InteractionProfile>(new OculusTouchInteractionProfile()));
	interactionProfiles.emplace_back(std::unique_ptr<InteractionProfile>(new KhrSimpleInteractionProfile()));
}
BaseInput::~BaseInput() = default;

EVRInputError BaseInput::SetActionManifestPath(const char* pchActionManifestPath)
{
	OOVR_LOGF("Loading manifest file '%s'", pchActionManifestPath);

	// Initialise the subaction path constants
	std::vector<std::string> subactionPathNames = {
		"/user/hand/left",
		"/user/hand/right",
	};

	allSubactionPaths.clear();
	for (const std::string& str : subactionPathNames) {
		XrPath path;
		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, str.c_str(), &path));
		allSubactionPaths.push_back(path);
	}

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
		std::unique_ptr<Action> actionPtr = std::make_unique<Action>();
		Action& action = *actionPtr;
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

		std::string name = action.fullName; // Array index may be evaluated first iirc, so pull this out now
		actions[name] = std::move(actionPtr);
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

		// In the OpenVR samples, they don't declare their action set. Are they automatically created?!
		// Also Vivecraft unfortunately does this, so we do have to support it.
		auto item = actionSets.find(action.setName);
		if (item == actionSets.end()) {
			OOVR_LOGF("Invalid action set '%s' for action '%s', creating implicit set", action.setName.c_str(), action.fullName.c_str());

			// Create this set
			ActionSet set{};
			set.name = action.setName;
			set.fullName = "/actions/" + set.name;
			set.usage = ActionSetUsage::LeftRight; // Just assume these default sets are in leftright mode, FIXME validate against steamvr
			actionSets[set.name] = std::make_unique<ActionSet>(set);

			// Now grab it and it should be there
			item = actionSets.find(action.setName);
			OOVR_FALSE_ABORT(item != actionSets.end());
		}

		action.set = item->second.get();
	}

	// Find the default bindings file
	// TODO load all of them, and let the OpenXR runtime choose which one to use
	std::string bestPath;
	int bestPriority = -1;
	for (Json::Value item : root["default_bindings"]) {
		std::string type = item["controller_type"].asString();

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

		if (priority > bestPriority) {
			bestPath = item["binding_url"].asString();
			bestPriority = priority;
		}
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
			OOVR_SOFT_ABORT("Warning: Unsupported action type - Skeleton"); // Not XR_STUBBED since AFAIK this didn't work before, and since OpenXR doesn't do skeletal stuff we'll have to sort this our ourselves
			break;
		default:
			OOVR_SOFT_ABORTF("Bad action type while remapping action %s: %d", act.fullName.c_str(), act.type);
		}

		// Listen on all the subactions
		info.subactionPaths = allSubactionPaths.data();
		info.countSubactionPaths = allSubactionPaths.size();

		OOVR_FAILED_XR_ABORT(xrCreateAction(act.set->xr, &info, &act.xr));
	}

	CreateLegacyActions();

	// Read the default bindings file, and load it into OpenXR
	for (const std::unique_ptr<InteractionProfile>& profile : interactionProfiles) {
		LoadBindingsSet(*profile);
	}

	// Attach everything to the current session
	BindInputsForSession();

	// Finish the setup for our VirtualInputs
	for (const auto& actionPair : actions) {
		const Action& action = *actionPair.second;
		for (const std::unique_ptr<VirtualInput>& input : action.virtualInputs) {
			input->PostInit();
		}
	}

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
	CreateLegacyActions();

	// Load in the suggested bindings for the legacy input actions
	for (const std::unique_ptr<InteractionProfile>& profile : interactionProfiles) {
		std::vector<XrActionSuggestedBinding> bindings;
		for (const auto& legacyController : legacyControllers) {
			profile->AddLegacyBindings(legacyController, bindings);
		}

		// Load the bindings into the runtime
		XrPath interactionProfilePath;
		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, profile->GetPath().c_str(), &interactionProfilePath));
		XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

		suggestedBindings.interactionProfile = interactionProfilePath;
		suggestedBindings.suggestedBindings = bindings.data();
		suggestedBindings.countSuggestedBindings = bindings.size();
		OOVR_FAILED_XR_ABORT(xrSuggestInteractionProfileBindings(xr_instance, &suggestedBindings));
	}

	// Attach everything to the current session
	BindInputsForSession();
}

void BaseInput::BindInputsForSession()
{
	OOVR_LOGF("Loading bindings file %s", bindingsPath.c_str());

	// Since the session has changed, any actionspaces we previously created are now invalid
	for (auto& pair : actions) {
		if (pair.second->actionSpace)
			pair.second->actionSpace = XR_NULL_HANDLE;
	}

	// Same goes for the actionspaces of the legacy controller pose actions
	for (LegacyControllerActions& lca : legacyControllers) {
		// No need to destroy it, the session it was attached to was destroyed
		lca.gripPoseSpace = XR_NULL_HANDLE;
		lca.aimPoseSpace = XR_NULL_HANDLE;
	}

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

void BaseInput::LoadBindingsSet(const struct InteractionProfile& profile)
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

	std::vector<XrActionSuggestedBinding> bindings;

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

		// TODO combine these loops for sources, poses and haptics

		for (const auto& srcJson : setJson["sources"]) {
			std::string importBasePath = lowerStr(srcJson["path"].asString());

			const Json::Value& inputsJson = srcJson["inputs"];
			for (const std::string& inputName : inputsJson.getMemberNames()) {
				const Json::Value item = inputsJson[inputName];

				std::string actionName = lowerStr(item["output"].asString());
				auto actionIter = actions.find(actionName);
				if (actionIter == actions.end())
					OOVR_ABORTF("Missing action '%s' in bindings file '%s'", actionName.c_str(), bindingsPath.c_str());
				Action& action = *actionIter->second;

				// There's probably some differences, but it looks like the SteamVR paths will 'just work' with OpenXR
				// FIXME this doesn't with with binding boolean actions to analogue inputs
				std::string pathStr = importBasePath + "/" + inputName;

				// Handle virtual paths - this creates the relevant virtual input for the specified path on this
				// action set and binds the Action to it. Note we don't want to cache and reuse the same virtual input
				// since if the runtime supports it the user may wish to rebind this action.
				const VirtualInputFactory* virtFactory = profile.GetVirtualInput(pathStr);
				if (virtFactory) {
					VirtualInput::BindInfo info = {};
					info.actionSet = action.set->xr;
					info.actionSetName = action.setName;
					info.openvrActionName = action.shortName;
					info.localisedName = action.shortName; // TODO localisation

					std::unique_ptr<VirtualInput> virt = virtFactory->BuildFor(info);
					virt->AddSuggestedBindings(bindings);
					action.virtualInputs.push_back(std::move(virt));

					// Note that we leave around the native xr instance, in case it's also bound to a native input later

					continue;
				}

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

		for (const auto& item : setJson["poses"]) {
			std::string specPath = lowerStr(item["path"].asString());

			std::string actionName = lowerStr(item["output"].asString());
			auto actionIter = actions.find(actionName);
			if (actionIter == actions.end())
				OOVR_ABORTF("Missing action '%s' in bindings file '%s'", actionName.c_str(), bindingsPath.c_str());
			const Action& action = *actionIter->second;

			// Translate over the paths - TODO find out what all the valid ones are
			std::string pathStr;
			if (specPath == "/user/hand/left/pose/raw") {
				pathStr = "/user/hand/left/input/aim/pose";
			} else if (specPath == "/user/hand/right/pose/raw") {
				pathStr = "/user/hand/right/input/aim/pose";
			} else {
				OOVR_LOGF("WARNING: Ignoring unknown pose path '%s'", specPath.c_str());
				continue;
			}

			if (!profile.IsInputPathValid(pathStr)) {
				OOVR_ABORTF("Built invalid input path %s from pose action %s", pathStr.c_str(), actionName.c_str());
			}

			XrPath path;
			OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, pathStr.c_str(), &path));
			bindings.push_back(XrActionSuggestedBinding{ action.xr, path });
		}

		for (const auto& item : setJson["haptics"]) {
			std::string pathStr = lowerStr(item["path"].asString());

			std::string actionName = lowerStr(item["output"].asString());
			auto actionIter = actions.find(actionName);
			if (actionIter == actions.end())
				OOVR_ABORTF("Missing haptic action '%s' in bindings file '%s'", actionName.c_str(), bindingsPath.c_str());
			const Action& action = *actionIter->second;

			if (!profile.IsInputPathValid(pathStr)) {
				OOVR_ABORTF("Built invalid input path %s from pose action %s", pathStr.c_str(), actionName.c_str());
			}

			XrPath path;
			OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, pathStr.c_str(), &path));
			bindings.push_back(XrActionSuggestedBinding{ action.xr, path });
		}
	}

	// If there aren't any bindings, that makes it simple
	// If we didn't return then xrSuggestInteractionProfileBindings would fail
	if (bindings.empty()) {
		return;
	}

	// Only register legacy bindings if this profile is supported by the application. This is to avoid the runtime picking
	// an input profile we know about but the game doesn't (eg khr/simple_controller) over one that the game has actually
	// defined bindings for (eg htc/vive_controller).
	for (const auto& legacyController : legacyControllers) {
		profile.AddLegacyBindings(legacyController, bindings);
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

void BaseInput::CreateLegacyActions()
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
		ctrl.handPath = "/user/hand/" + side;
		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, ctrl.handPath.c_str(), &ctrl.handPathXr));

		auto create = [&](XrAction* out, const std::string& name, const std::string& englishName, XrActionType type) {
			XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };
			info.actionType = type;

			std::string codeName = "legacy-" + side + "-" + name;
			strcpy_arr(info.actionName, codeName.c_str());

			std::string fullHumanName = "Legacy input: " + englishName + " - " + side; // TODO localisation
			strcpy_arr(info.localizedActionName, fullHumanName.c_str());

			OOVR_FAILED_XR_ABORT(xrCreateAction(legacyInputsSet, &info, out));
		};

		create(&ctrl.system, "system", "System Button", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.menu, "menu", "Menu Button", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.menuTouch, "menu-touch", "Menu Button (Touch)", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.btnA, "btn-a", "'A' Button", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.btnATouch, "btn-a-touch", "'A' Button (Touch)", XR_ACTION_TYPE_BOOLEAN_INPUT);

		create(&ctrl.stickBtn, "thumbstick-btn", "Thumbstick Button", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.stickBtnTouch, "thumbstick-btn-touch", "Thumbstick Button (Touch)", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.stickX, "thumbstick-x", "Thumbstick X axis", XR_ACTION_TYPE_FLOAT_INPUT);
		create(&ctrl.stickY, "thumbstick-y", "Thumbstick Y axis", XR_ACTION_TYPE_FLOAT_INPUT);

		// Note that while we define the grip as a float, we can still bind it to boolean actions and the OpenXR runtime will
		// return 0.0 or 1.0 depending on the button status. OpenXR 1.0 § 11.4.
		create(&ctrl.grip, "grip", "Grip", XR_ACTION_TYPE_FLOAT_INPUT);
		create(&ctrl.trigger, "trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT);
		create(&ctrl.triggerTouch, "trigger-touch", "Trigger (Touch)", XR_ACTION_TYPE_BOOLEAN_INPUT);

		create(&ctrl.haptic, "haptic", "Vibration Haptics", XR_ACTION_TYPE_VIBRATION_OUTPUT);

		create(&ctrl.gripPoseAction, "grip-pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT);
		create(&ctrl.aimPoseAction, "aim-pose", "Aim Pose", XR_ACTION_TYPE_POSE_INPUT);
	}
}

EVRInputError BaseInput::GetActionSetHandle(const char* pchActionSetName, VRActionSetHandle_t* pHandle)
{
	*pHandle = k_ulInvalidActionSetHandle;

	std::string prefix = "/actions/";
	if (strncmp(prefix.c_str(), pchActionSetName, prefix.size()) != 0) {
		OOVR_SOFT_ABORTF("Invalid action set name '%s' - missing or bad prefix", pchActionSetName);
		// This is a bogus error, SteamVR will just make a new handle
		return vr::VRInputError_NameNotFound;
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
	*(uint64_t*)pHandle = 0;

	ITrackedDevice::HandType handType;

	if (!strcmp("/user/hand/left", pchInputSourcePath)) {
		handType = ITrackedDevice::HAND_LEFT;
	} else if (!strcmp("/user/hand/right", pchInputSourcePath)) {
		handType = ITrackedDevice::HAND_RIGHT;
	} else {
		// FIXME you can specify a path like /user/hand/left/input/select/click
		OOVR_SOFT_ABORTF("Unknown input source '%s'", pchInputSourcePath);
		return VRInputError_NameNotFound; // Probably not valid in this context, but whatever
	}

	for (vr::TrackedDeviceIndex_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		ITrackedDevice* dev = BackendManager::Instance().GetDevice(i);
		if (dev && dev->GetHand() == handType) {
			*pHandle = devToIVH(i);
			return VRInputError_None;
		}
	}

	OOVR_ABORTF("Missing device for input source %d '%s'", handType, pchInputSourcePath);
}

EVRInputError BaseInput::UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets,
    uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount)
{
	// TODO if the game is using legacy input, call this every frame

	OOVR_FALSE_ABORT(sizeof(*pSets) == unSizeOfVRSelectedActionSet_t);

	// First tell all the VirtualInputs to update, to process bChanged
	for (const auto& actionPair : actions) {
		const Action& action = *actionPair.second;
		for (const std::unique_ptr<VirtualInput>& input : action.virtualInputs) {
			input->OnPreFrame();
		}
	}

	// Make sure all the ActionSets have the same priority, since we don't have any way around that right now
	if (unSetCount > 1) {
		int priority = pSets[0].nPriority;
		for (int i = 1; i < unSetCount; i++) {
			if (pSets[i].nPriority != priority) {
				ActionSet* as1 = cast_ASH(pSets[0].ulActionSet);
				ActionSet* curAs = cast_ASH(pSets[1].ulActionSet);
				OOVR_ABORTF("Active action set %s (%d) and %s (%d) have different priorities, this is not yet supported",
				    as1->fullName.c_str(), curAs->fullName.c_str());
			}
		}
	}

	std::vector<XrActiveActionSet> aas(unSetCount + 1);

	for (int i = 0; i < unSetCount; i++) {
		VRActiveActionSet_t& set = pSets[i];

		ActionSet* as = cast_ASH(set.ulActionSet);
		aas[i].actionSet = as->xr;

		if (set.ulRestrictedToDevice != vr::k_ulInvalidInputValueHandle) {
			OOVR_ABORTF("Active action set %s has ulRestrictedToDevice set, not yet implemented",
			    as->fullName.c_str());

			// Once we've got something to test it with, it should look something like this:
			// index = cast_IVH(set.ulRestrictedToDevice);
			// ITrackedDevice::HandType hand = dev->GetHand();
			// LegacyControllerActions& ctrl = legacyControllers[hand];
			// aas[i].subactionPath = ctrl.pathXr;
		}
	}

	// Ad the last set, the legacy input set
	aas.at(unSetCount).actionSet = legacyInputsSet;

	XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
	syncInfo.activeActionSets = aas.data();
	syncInfo.countActiveActionSets = aas.size();
	OOVR_FAILED_XR_ABORT(xrSyncActions(xr_session, &syncInfo));
	syncSerial++;

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
	syncSerial++;
}

EVRInputError BaseInput::GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	Action* act = cast_AH(action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = act->xr;

	// Unfortunately to implement activeOrigin we have to loop through and query each action state
	for (XrPath subactionPath : allSubactionPaths) {
		// TODO what's the performance cost of this?
		VRInputValueHandle_t ao = activeOriginToIVH(subactionPath);
		if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle && ao != ulRestrictToDevice)
			continue;

		getInfo.subactionPath = subactionPath;
		XrActionStateBoolean state = { XR_TYPE_ACTION_STATE_BOOLEAN };
		OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session, &getInfo, &state));

		// If the subaction isn't set, or it was set but not active, or it was set
		// but the state was false and it's not now, then override it.
		if (!(state.isActive > pActionData->bActive || state.currentState > pActionData->bState))
			continue;

		pActionData->bState = state.currentState;
		pActionData->bActive = state.isActive;
		pActionData->bChanged = state.changedSinceLastSync;
		// TODO implement fUpdateTime
		pActionData->activeOrigin = ao;
	}

	// Check the virtual inputs
	for (const auto& virt : act->virtualInputs) {
		OOVR_InputDigitalActionData_t tmp = {};
		virt->GetDigitalActionData(&tmp);

		if (tmp.bActive > pActionData->bActive || tmp.bState > pActionData->bState)
			*pActionData = tmp;
	}

	// Note it's possible we didn't set any output if this action isn't bound to anything, just leave the
	//  struct at it's default values.

	return VRInputError_None;
}

EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	Action* act = cast_AH(action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	// TODO implement ulRestrictToDevice
	OOVR_FALSE_ABORT(ulRestrictToDevice == vr::k_ulInvalidInputValueHandle);

	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = act->xr;

	switch (act->type) {
	case ActionType::Vector1: {
		XrActionStateFloat state = { XR_TYPE_ACTION_STATE_FLOAT };
		OOVR_FAILED_XR_ABORT(xrGetActionStateFloat(xr_session, &getInfo, &state));

		pActionData->x = state.currentState;
		pActionData->y = 0;
		pActionData->z = 0;
		pActionData->bActive = state.isActive;
		break;
	}
	case ActionType::Vector2: {
		XrActionStateVector2f state = { XR_TYPE_ACTION_STATE_VECTOR2F };
		OOVR_FAILED_XR_ABORT(xrGetActionStateVector2f(xr_session, &getInfo, &state));

		pActionData->x = state.currentState.x;
		pActionData->y = state.currentState.y;
		pActionData->z = 0;
		pActionData->bActive = state.isActive;
		break;
	}
	case ActionType::Vector3:
		OOVR_ABORTF("Input type vector3 unsupported: %s", act->fullName.c_str());
		break;
	default:
		OOVR_ABORTF("Invalid action type %d for action %s", act->type, act->fullName.c_str());
		break;
	}

	// TODO implement the deltas
	// TODO implement activeOrigin

	return VRInputError_None;
}

EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
    InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	Action* act = cast_AH(action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	// TODO test ulRestrictToDevice
	OOVR_FALSE_ABORT(ulRestrictToDevice == vr::k_ulInvalidInputValueHandle);

	if (act->type != ActionType::Pose)
		OOVR_ABORTF("Invalid action type %d for action %s", act->type, act->fullName.c_str());

	// Create the action space if it doesn't already exist
	if (!act->actionSpace) {
		XrActionSpaceCreateInfo info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		info.poseInActionSpace = S2O_om34_pose(G2S_m34(glm::identity<glm::mat4>()));
		info.action = act->xr;
		OOVR_FAILED_XR_ABORT(xrCreateActionSpace(xr_session, &info, &act->actionSpace));
	}

	// Unfortunately to implement activeOrigin we have to loop through and query each action state
	for (XrPath subactionPath : allSubactionPaths) {
		// TODO what's the performance cost of this?
		VRInputValueHandle_t ao = activeOriginToIVH(subactionPath);
		if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle && ao != ulRestrictToDevice)
			continue;

		// Get the info, which only says if it's active or not
		XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
		getInfo.action = act->xr;
		// Note we can't cleanly set subactionPath here, since the returned pose may be incorrect if the
		//  runtime made the space from the other controller. It's unlikely to ever be a significant issue
		//  though since it'll only occur if the pose is bound to multiple inputs.
		// Anyway we do have to have this, otherwise this will read from either controller's state and
		//  we'll always read the first device as the active origin.
		getInfo.subactionPath = subactionPath;
		XrActionStatePose state = { XR_TYPE_ACTION_STATE_POSE };
		OOVR_FAILED_XR_ABORT(xrGetActionStatePose(xr_session, &getInfo, &state));

		pActionData->bActive = state.isActive;
		pActionData->activeOrigin = ao;

		if (state.isActive) {
			xr_utils::PoseFromSpace(&pActionData->pose, act->actionSpace, eOrigin);
		}

		// TODO implement the deltas

		// Stop as soon as we find the first available input
		if (state.isActive)
			break;
	}

	return VRInputError_None;
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
	Action* act = cast_AH(action);

	if (act->type != ActionType::Vibration) {
		OOVR_ABORTF("Cannot trigger vibration on non-vibration (type=%d) action '%s'", act->type, act->fullName.c_str());
	}

	// TODO check the subaction stuff works properly
	XrPath subactionPath = XR_NULL_PATH;
	if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle) {
		TrackedDeviceIndex_t idx = cast_IVH(ulRestrictToDevice);
		ITrackedDevice* dev = BackendManager::Instance().GetDevice(idx);
		if (dev && dev->GetHand() != ITrackedDevice::HAND_NONE) {
			LegacyControllerActions& ctrl = legacyControllers[dev->GetHand()];
			subactionPath = ctrl.handPathXr;
		}
	}

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action = act->xr;
	info.subactionPath = subactionPath;

	// TODO implement fStartSecondsFromNow - just assume most games leave it at or very close to zero

	XrHapticVibration vibration = { XR_TYPE_HAPTIC_VIBRATION };
	vibration.frequency = XR_FREQUENCY_UNSPECIFIED; // TODO we should maybe implement this?
	vibration.duration = (int)(fDurationSeconds * 1000000000.0f);
	vibration.amplitude = fAmplitude;

	OOVR_FAILED_XR_ABORT(xrApplyHapticFeedback(xr_session, &info, (XrHapticBaseHeader*)&vibration));

	return VRInputError_None;
}

EVRInputError BaseInput::GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle,
    VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t* originsOut, uint32_t originOutCount)
{
	ActionSet* set = cast_ASH(actionSetHandle);
	Action* act = cast_AH(digitalActionHandle);

	// TODO find something that passes in non-matching values and see what results it wants, or try it with SteamVR
	if (act->set != set) {
		OOVR_ABORTF("GetActionOrigins: set mismatch %s vs %s", set->name.c_str(), act->fullName.c_str());
	}

	ZeroMemory(originsOut, originOutCount * sizeof(*originsOut));

	// Go through both the real action and any virtual actions, and add them all together
	std::vector<XrAction> relevantActions;
	relevantActions.emplace_back(act->xr);

	for (const std::unique_ptr<VirtualInput>& virt : act->virtualInputs) {
		std::vector<XrAction> virtActions = virt->GetActionsForOriginLookup();
		relevantActions.insert(relevantActions.end(), virtActions.begin(), virtActions.end());
	}

	std::set<std::string> sources;
	for (XrAction action : relevantActions) {
		XrBoundSourcesForActionEnumerateInfo info = { XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO };
		info.action = action;

		// 20 will be more than enough, saves a second call
		XrPath tmp[20];
		uint32_t count;
		OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session, &info, ARRAYSIZE(tmp), &count, tmp));

		// Now for each source find the /user/hand/abc substring that it starts with
		char buff[XR_MAX_PATH_LENGTH + 1];
		for (int i = 0; i < count; i++) {
			uint32_t len;
			OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, tmp[i], XR_MAX_PATH_LENGTH, &len, buff));

			std::string path(buff, len);
			int endOfHandPos = path.find('/', strlen("/user/hand/") + 1);
			path.erase(endOfHandPos);
			sources.insert(std::move(path));
		}
	}

	// Copy out the sources
	int i = 0;
	for (const std::string& path : sources) {
		if (i >= originOutCount)
			return vr::VRInputError_MaxCapacityReached; // TODO check this is correct
		GetInputSourceHandle(path.c_str(), &originsOut[i]);
		i++;
	}

	return VRInputError_None;
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

EVRInputError BaseInput::GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t* info, uint32_t unOriginInfoSize)
{
	memset(info, 0, unOriginInfoSize);
	OOVR_FALSE_ABORT(unOriginInfoSize == sizeof(InputOriginInfo_t));

	TrackedDeviceIndex_t dev = cast_IVH(origin);

	info->trackedDeviceIndex = dev;
	info->devicePath = origin; // TODO is this how it's supposed to work?
	strcpy_arr(info->rchRenderModelComponentName, "getorigintrackeddeviceinfo_testing"); // TODO figure out how this should work

	return VRInputError_None;
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

TrackedDeviceIndex_t BaseInput::cast_IVH(VRInputValueHandle_t handle)
{
	if (handle == vr::k_ulInvalidInputValueHandle)
		OOVR_ABORT("Called cast_IVH for invalid input value handle");

	// -1000 to undo what we added - see GetInputSourceHandle
	TrackedDeviceIndex_t dev = ((TrackedDeviceIndex_t)handle) - 1000;

	// Make sure it's a valid handle
	if (dev < 0 || dev > vr::k_unMaxTrackedDeviceCount) {
		OOVR_ABORTF("Corrupt VRInputValueHandle - value %d", handle);
	}

	return dev;
}

VRInputValueHandle_t BaseInput::devToIVH(vr::TrackedDeviceIndex_t index)
{
	// Add 1000 so any valid device doesn't equal k_ulInvalidInputValueHandle
	return index + 1000;
}

VRInputValueHandle_t BaseInput::activeOriginToIVH(XrPath path)
{
	if (path == XR_NULL_PATH)
		return vr::k_ulInvalidInputValueHandle;

	// Find the hand for this path
	int ctrlId = -1;
	for (int hand = 0; hand < ARRAYSIZE(legacyControllers); hand++) {
		if (legacyControllers[hand].handPathXr == path) {
			ctrlId = hand;
			break;
		}
	}

	if (ctrlId == -1) {
		uint32_t len;
		OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, path, 0, &len, nullptr));
		std::vector<char> str(len);
		OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, path, len, &len, str.data()));
		OOVR_ABORTF("Unknown active origin path '%s'", str.data());
	}

	// Convert it into a device index
	for (vr::TrackedDeviceIndex_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		if (DeviceIndexToHandId(i) == ctrlId)
			return devToIVH(i);
	}

	OOVR_ABORTF("Cannot find controller tracking ID by handId=%d", ctrlId);
}

VRInputValueHandle_t BaseInput::HandPathToIVH(const std::string& path)
{
	XrPath xrPath;
	OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, path.c_str(), &xrPath));
	return activeOriginToIVH(xrPath);
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
