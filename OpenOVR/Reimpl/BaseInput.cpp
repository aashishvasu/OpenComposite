#include "generated/interfaces/vrtypes.h"
#include "logging.h"
#include "openxr/openxr.h"
#include "stdafx.h"
#define BASE_IMPL
#include "BaseInput.h"
#include <string>

#include <convert.h>

#include "Drivers/Backend.h"
#include "generated/static_bases.gen.h"
#include <algorithm>
#include <cmath>
#include <codecvt>
#include <fstream>
#include <glm/gtc/matrix_inverse.hpp>
#include <locale>
#include <map>
#include <math.h>
#include <optional>
#include <set>
#include <utility>

#include "Misc/xrmoreutils.h"

// Use RenderModels for the pose offsets, which are the same as component positions
#include "BaseRenderModels.h"

using namespace vr;

#include "../DrvOpenXR/XrBackend.h"

// On Android, the application must supply a function to load the contents of a file
#include "Misc/android_api.h"

/**
 * Macro for creating an Action object from a handle and verifying isn't invalid.
 * If it is invalid, it will cause the surrounding function to return VRInputError_InvalidHandle.
 * action_var_name is the name of the resulting Action object if the handle isn't invalid.
 */
#define GET_ACTION_FROM_HANDLE(action_var_name, handle)              \
	Action* action_var_name = cast_AH(handle);                       \
	do {                                                             \
		if (!action_var_name) {                                      \
			OOVR_LOG_ONCE("WARNING: Invalid action handle passed!"); \
			return VRInputError_InvalidHandle;                       \
		}                                                            \
	} while (0)

/**
 * Same as GET_ACTION_FROM_HANDLE but for ActionSets.
 */
#define GET_ACTION_SET_FROM_HANDLE(action_var_name, handle)          \
	ActionSet* action_var_name = cast_ASH(handle);                   \
	do {                                                             \
		if (!action_var_name) {                                      \
			OOVR_LOG_ONCE("WARNING: Invalid action handle passed!"); \
			return VRInputError_InvalidHandle;                       \
		}                                                            \
	} while (0)

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
BaseInput::Action::~Action() = default;
BaseInput::ActionSource::~ActionSource() = default;
BaseInput::InputValueHandle::InputValueHandle() = default;
BaseInput::InputValueHandle::~InputValueHandle() = default;

// Registry implementation

template <typename T>
BaseInput::Registry<T>::~Registry() = default;
template <typename T>
BaseInput::Registry<T>::Registry(uint32_t _maxNameSize)
    : maxNameSize(_maxNameSize) {}

template <typename T>
T* BaseInput::Registry<T>::LookupItem(const std::string& name) const
{
	auto iter = itemsByName.find(lowerStr(name));
	if (iter == itemsByName.end())
		return nullptr;
	return iter->second;
}

template <typename T>
T* BaseInput::Registry<T>::LookupItem(RegHandle handle) const
{
	auto iter = itemsByHandle.find(handle);
	if (iter == itemsByHandle.end())
		return nullptr;
	return iter->second;
}

template <typename T>
BaseInput::RegHandle BaseInput::Registry<T>::LookupHandle(const std::string& name)
{
	std::string lowerName = ShortenOrLookupName(name);
	auto iter = handlesByName.find(lowerName);
	if (iter != handlesByName.end())
		return iter->second;

	// Create a new dummy handle. It's only in the handlesByName map, so looking up the item
	// for it will return nullptr.
	RegHandle handle = 0xabcd0001 + handlesByName.size();
	handlesByName[lowerName] = handle;
	namesByHandle[handle] = name;
	return handle;
}

template <typename T>
std::string BaseInput::Registry<T>::ShortenOrLookupName(const std::string& longName)
{
	std::string ret = lowerStr(longName);
	if (ret.size() > maxNameSize - 1) {
		auto iter = longNames.find(ret);
		if (iter == longNames.end()) {
			// new name - shorten and append "_ln" + unique number (ln for Long Name)
			std::string fullName = ret;
			std::string unique_id = "_ln" + std::to_string(longNames.size());
			ret = ret.substr(0, maxNameSize - 1 - unique_id.size()) + unique_id;
			longNames[fullName] = ret;
			OOVR_LOGF("Shortened name %s to %s", fullName.c_str(), ret.c_str());
		} else {
			// name has already been shortened before - find the shortened version
			auto iter2 = handlesByName.find(iter->second);

			// if it's in the longNames map, it has to be in the handlesByName map
			// otherwise something probably went wrong
			OOVR_FALSE_ABORT(iter2 != handlesByName.end());

			// return shortened name
			return iter2->first;
		}
	}
	return ret;
}

template <typename T>
T* BaseInput::Registry<T>::Initialise(const std::string& name, std::unique_ptr<T> value)
{
	std::string lowerName = lowerStr(name);
	RegHandle handle = 0;

	// apparently games CAN in fact grab handles before initialization - Kayak VR does this
	// grab already created handle if it exists
	if (handlesByName.count(lowerName) != 0) {
		// since we only generate dummy handles before initialization, make sure we only have dummy handles
		// dummy handles have no associated items
		handle = handlesByName.find(lowerName)->second;
		OOVR_FALSE_ABORT(itemsByHandle.find(handle) == itemsByHandle.end());
	}

	// Move the pointer into storage, so we'll own it
	T* ptr = value.get();
	storage.emplace_back(std::move(value));

	// Use pointer as handle, for ease of debugging only
	// Since our values live as long as the registry, it's guaranteed their addresses won't repeat
	if (!handle) {
		handle = (RegHandle)ptr;
		handlesByName[lowerName] = handle;
		namesByHandle[handle] = lowerName;
	}

	itemsByName[lowerName] = ptr;
	itemsByHandle[handle] = ptr;

	// Convenience return
	return ptr;
}

template <typename T>
void BaseInput::Registry<T>::Reset()
{
	// We want to preserve handlesByName and namesByHandle, because handles are supposed to always be accessible
	// from the same values regardless of if said handles are actually currently valid
	// In the case of NomaiVR, it will set an action manifest, get all the action handles, and then set another (identical) manifest
	// We can clear the actual item storage though, since these will no longer be valid
	itemsByHandle.clear();
	itemsByName.clear();
	storage.clear();
}

// ---

BaseInput::BaseInput()
    : actionSets(XR_MAX_ACTION_SET_NAME_SIZE), actions(XR_MAX_ACTION_NAME_SIZE)
{
	// Initialise the subaction path constants
	for (const std::string& str : allSubactionPathNames) {
		XrPath path;
		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, str.c_str(), &path));
		allSubactionPaths.push_back(path);
	}
}
BaseInput::~BaseInput()
{
	for (XrHandTrackerEXT& handTracker : handTrackers) {
		if (handTracker != XR_NULL_HANDLE)
			xr_ext->xrDestroyHandTrackerEXT(handTracker);
		handTracker = XR_NULL_HANDLE;
	}
}

EVRInputError BaseInput::SetActionManifestPath(const char* pchActionManifestPath)
{
	OOVR_LOGF("Loading manifest file '%s'", pchActionManifestPath);

	//////////////
	//// Load the actions from the manifest file
	//////////////

	if (hasLoadedActions) {
		// It stands to reason that nothing would happen if we try to load the same manifest again
		if (loadedActionsPath == pchActionManifestPath)
			return vr::VRInputError_None;

		OOVR_LOG("Received another manifest! Restarting session to reattach inputs...");
		for (std::unique_ptr<ActionSet>& as : actionSets.GetItems()) {
			OOVR_FAILED_XR_ABORT(xrDestroyActionSet(as->xr));
		}
		OOVR_FAILED_XR_ABORT(xrDestroyActionSet(legacyInputsSet));
		legacyInputsSet = XR_NULL_HANDLE;
		actions.Reset();
		actionSets.Reset();
		DpadBindingInfo::parents.clear();
	}

	restartingSession = true;
	XrBackend::MaybeRestartForInputs();
	restartingSession = false;

	hasLoadedActions = true;
	loadedActionsPath = pchActionManifestPath;

	Json::Value root;
	// It says 'open or parse', but really it ignores parse errors - TODO catch those
	if (!ReadJson(utf8to16(pchActionManifestPath), root))
		OOVR_ABORT("Failed to open or parse input manifest");

	// Random setting which is in the action manifest
	allowSetDominantHand = root["supports_dominant_hand_setting"].asBool();

	// Parse the actions
	OOVR_LOG("Parsing actions...");
	for (Json::Value item : root["actions"]) {
		std::unique_ptr<Action> actionPtr = std::make_unique<Action>();
		Action& action = *actionPtr;

		action.fullName = actions.ShortenOrLookupName(item["name"].asString());

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

		if (actions.LookupItem(action.fullName) != nullptr)
			OOVR_ABORTF("Duplicate action name '%s'", action.fullName.c_str());

		// If this is a skeletal action, see what hand it's bound to. Uniquely, this is done in the actions manifest itself
		if (action.type == ActionType::Skeleton) {
			// Default to the left hand, so we can always safely use it as a 0 or 1 value for indexing arrays etc
			action.skeletalHand = ITrackedDevice::HAND_LEFT;

			std::string skelSide = item["skeleton"].asString();
			if (skelSide.empty()) {
				OOVR_SOFT_ABORTF("Skeletal hand side not set for action '%s'", action.fullName.c_str());
			} else if (skelSide == "/skeleton/hand/left") {
				action.skeletalHand = ITrackedDevice::HAND_LEFT;
			} else if (skelSide == "/skeleton/hand/right") {
				action.skeletalHand = ITrackedDevice::HAND_RIGHT;
			} else {
				OOVR_SOFT_ABORTF("Unknown/unsupported skeletal hand side '%s' for action '%s'",
				    skelSide.c_str(), action.fullName.c_str());
			}
		}

		std::string name = action.fullName; // Array index may be evaluated first iirc, so pull this out now
		actions.Initialise(name, std::move(actionPtr));
	}

	// Parse the action sets
	OOVR_LOG("Parsing action sets...");
	for (Json::Value item : root["action_sets"]) {
		ActionSet set = {};

		set.fullName = actionSets.ShortenOrLookupName(item["name"].asString());

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
		actionSets.Initialise(set.fullName, std::make_unique<ActionSet>(set));
	}

	// Make sure all the actions have a corresponding set
	for (const std::unique_ptr<Action>& action : actions.GetItems()) {
		// In the OpenVR samples, they don't declare their action set. Are they automatically created?!
		// Also Vivecraft unfortunately does this, so we do have to support it.
		std::string fullName = "/actions/" + action->setName;
		ActionSet* set = actionSets.LookupItem(fullName);
		if (set == nullptr) {
			OOVR_LOGF("Invalid action set '%s' for action '%s', creating implicit set", action->setName.c_str(), action->fullName.c_str());

			// Create this set
			set = actionSets.Initialise(fullName, std::make_unique<ActionSet>());
			set->name = action->setName;
			set->fullName = fullName;
			set->usage = ActionSetUsage::LeftRight; // Just assume these default sets are in leftright mode, FIXME validate against steamvr
		}

		action->set = set;
	}

	//////////////////////////
	/// Now we've got everything done, load the actions into OpenXR
	//////////////////////////

	for (const std::unique_ptr<ActionSet>& as : actionSets.GetItems()) {
		XrActionSetCreateInfo createInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
		std::string safeName = escapePathString(as->name);
		strcpy_arr(createInfo.actionSetName, safeName.c_str());
		strcpy_arr(createInfo.localizedActionSetName, as->name.c_str()); // TODO localisation

		OOVR_FAILED_XR_ABORT(xrCreateActionSet(xr_instance, &createInfo, &as->xr));
	}

	for (const std::unique_ptr<Action>& act : actions.GetItems()) {
		XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };
		std::string safeName = escapePathString(act->shortName);
		strcpy_arr(info.actionName, safeName.c_str());
		strcpy_arr(info.localizedActionName, act->shortName.c_str()); // TODO localisation

		switch (act->type) {
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
			// Poses are special, since they have offsets
			continue;
		case ActionType::Skeleton:
			// Don't actually create an action for skeletons, since we'll get their data from the hand tracking extension.
			continue;
		default:
			OOVR_SOFT_ABORTF("Bad action type while remapping action %s: %d", act->fullName.c_str(), act->type);
		}

		// Listen on all the subactions
		info.subactionPaths = allSubactionPaths.data();
		info.countSubactionPaths = allSubactionPaths.size();

		OOVR_FAILED_XR_ABORT(xrCreateAction(act->set->xr, &info, &act->xr));
	}

	CreateLegacyActions();

	// load all available bindings for known interaction profiles
	OOVR_LOG("Loading known bindings...");
	// Map of OpenVR names (i.e. "vive_controller") to interaction profile pointers
	std::unordered_map<std::string, InteractionProfile*> OpenVRNameToProfile;
	for (const std::unique_ptr<InteractionProfile>& profile : InteractionProfile::GetProfileList()) {
		if (profile->GetOpenVRName().has_value())
			OpenVRNameToProfile[profile->GetOpenVRName().value()] = profile.get();
		else
			// Add any unrecognized profiles (simple controller) to the end so it still gets assigned a profile
			OpenVRNameToProfile[profile->GetPath()] = profile.get();
	}

	// For determining the best path for supported controllers that don't have a profile handled by the game (i.e. khr/simple_controller)
	std::string backupPath;
	int8_t priority = -1;

	// these priorities are a bit arbitrary
	const std::unordered_map<std::string, uint8_t> typeToPriority = {
		{ "oculus_touch", 3 },
		{ "knuckles", 2 },
		{ "generic", 1 }
	};

	for (Json::Value item : root["default_bindings"]) {
		std::string controller_type = item["controller_type"].asString();
		std::string path = dirnameOf(pchActionManifestPath) + "/" + item["binding_url"].asString();

		// look if we know about this OpenVR name
		// have to loop through every binding type because default_bindings is an array for some reason
		auto iter = OpenVRNameToProfile.find(controller_type);
		if (iter != OpenVRNameToProfile.end()) {
			LoadBindingsSet(*(iter->second), path);
			// profile has been bound: we don't need it in our map anymore
			OpenVRNameToProfile.erase(iter);
		}

		// get priority for remaining unbound profiles
		auto iter2 = typeToPriority.find(controller_type);
		if (iter2 != typeToPriority.end()) {
			if (iter2->second > priority) {
				priority = iter2->second;
				backupPath = path;
			}
		} else if (priority < 0) {
			priority = 0;
			backupPath = path;
		}
	}

	// remaining profiles: bind to our backup path
	for (auto& [name, profile_ptr] : OpenVRNameToProfile) {
		LoadBindingsSet(*profile_ptr, backupPath);
	}

	// Attach everything to the current session
	BindInputsForSession();
	return vr::VRInputError_None;
}

void BaseInput::LoadEmptyManifestIfRequired()
{
	if (hasLoadedActions)
		return;

	restartingSession = true;
	XrBackend::MaybeRestartForInputs();
	restartingSession = false;

	OOVR_LOG("Loading virtual empty manifest");

	hasLoadedActions = true;
	usingLegacyInput = true;

	// TODO deduplicate with the regular manifest loader
	CreateLegacyActions();

	// Load in the suggested bindings for the legacy input actions
	for (const std::unique_ptr<InteractionProfile>& profile : InteractionProfile::GetProfileList()) {
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
	// This is called from DrvOpenXR::SetupSession. If we requested a session restart ourselves, we're also
	// going to bind the inputs ourselves anyway, so we don't want to do that twice.
	// If we didn't request the restart but we also haven't even loaded actions yet, there's nothing to bind yet.
	if (restartingSession || !hasLoadedActions)
		return;

	// Since the session has changed, any actionspaces we previously created are now invalid
	for (const std::unique_ptr<Action>& action : actions.GetItems()) {
		action->actionSpaces.clear();
	}

	// Same goes for the actionspaces of the legacy controller pose actions, this time create
	// new ones for this session.
	for (LegacyControllerActions& lca : legacyControllers) {
		// No need to destroy it, the session it was attached to was destroyed
		lca.gripPoseSpace = XR_NULL_HANDLE;
		lca.aimPoseSpace = XR_NULL_HANDLE;

		// Create the new spaces
		XrActionSpaceCreateInfo info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		info.poseInActionSpace = S2O_om34_pose(G2S_m34(glm::identity<glm::mat4>()));
		info.action = lca.gripPoseAction;
		OOVR_FAILED_XR_ABORT(xrCreateActionSpace(xr_session.get(), &info, &lca.gripPoseSpace));
		info.action = lca.aimPoseAction;
		OOVR_FAILED_XR_ABORT(xrCreateActionSpace(xr_session.get(), &info, &lca.aimPoseSpace));
	}

	// Note: even if actionSets is empty, we always still want to load the legacy set.

	// Now attach the action sets to the OpenXR session, making them immutable (including attaching suggested bindings)
	std::vector<XrActionSet> sets;
	for (const std::unique_ptr<ActionSet>& as : actionSets.GetItems()) {
		sets.push_back(as->xr);
	}

	sets.push_back(legacyInputsSet);

	XrSessionActionSetsAttachInfo attachInfo = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attachInfo.actionSets = sets.data();
	attachInfo.countActionSets = sets.size();
	OOVR_FAILED_XR_ABORT(xrAttachSessionActionSets(xr_session.get(), &attachInfo));

	// Setup hand tracking if supported
	if (xr_gbl->handTrackingProperties.supportsHandTracking) {
		XrHandTrackerCreateInfoEXT createInfo = { XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
		for (int i = 0; i < 2; i++) {
			createInfo.hand = i == 0 ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
			createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
			OOVR_FAILED_XR_ABORT(xr_ext->xrCreateHandTrackerEXT(xr_session.get(), &createInfo, &handTrackers[i]));
		}
	}
}

void BaseInput::LoadBindingsSet(const struct InteractionProfile& profile, const std::string& bindingsPath)
{
	OOVR_LOGF("Loading bindings for %s", profile.GetPath().c_str());
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

		ActionSet* set = actionSets.LookupItem("/actions/" + setName);
		if (set == nullptr) {
			// It seems ActionSets specified in the bindings but not in the action manifest are just ignored.
			// Jet Island has those as of 29/05/2022.
			OOVR_LOGF("WARNING: Missing action set '%s' in bindings file '%s', ignoring", setName.c_str(), bindingsPath.c_str());
			continue;
		}

		// TODO combine these loops for sources, poses and haptics

		for (const auto& srcJson : setJson["sources"]) {
			std::string importBasePath = lowerStr(srcJson["path"].asString());

			const Json::Value& inputsJson = srcJson["inputs"];
			for (const std::string& inputName : inputsJson.getMemberNames()) {
				const Json::Value item = inputsJson[inputName];

				std::string actionName = actions.ShortenOrLookupName(item["output"].asString());
				Action* action = actions.LookupItem(actionName);

				// For some reason, No Man's Sky has actions that don't exist in the manifest in its binding files
				if (action == nullptr) {
					OOVR_LOGF("WARNING: Action '%s' (from binding file %s) does not exist in manifest, skipping", actionName.c_str(), bindingsPath.c_str());
					continue;
				}

				if (srcJson["mode"].asString() == "dpad") {
					LoadDpadAction(profile, importBasePath, inputName, srcJson["parameters"]["sub_mode"].asString(), action, bindings);
					continue;
				}

				// Translate path string to an appropriate path supported by this interaction profile, if necessary
				std::string pathStr;
				// special case: "position" in OpenVR is a 2D vector, in OpenXR this can be represented by the parent of the input value
				// See the OpenXR spec section 11.4 ("Suggested Bindings")
				if (inputName == "position") {
					pathStr = profile.TranslateAction(importBasePath);
				} else {
					pathStr = profile.TranslateAction(importBasePath + "/" + inputName);
				}

				if (!profile.IsInputPathValid(pathStr)) {
					OOVR_LOGF("WARNING: Built invalid input path %s for profile %s from action %s, skipping",
					    pathStr.c_str(), profile.GetPath().c_str(), actionName.c_str());
					continue;
				}

				XrPath path;
				OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, pathStr.c_str(), &path));
				bindings.push_back(XrActionSuggestedBinding{ action->xr, path });
			}
		}

		for (const auto& item : setJson["poses"]) {
			std::string specPath = lowerStr(item["path"].asString());

			std::string actionName = lowerStr(item["output"].asString());
			Action* action = actions.LookupItem(actionName);
			if (action == nullptr)
				OOVR_ABORTF("Missing action '%s' in bindings file '%s'", actionName.c_str(), bindingsPath.c_str());

			PoseBindingInfo info;

			// Find which hand this is from - on a path like /user/hand/left/pose/raw, after this
			// we're left with pose/raw.
			std::string withoutPrefix = specPath;
			info.hand = ParseAndRemoveHandPrefix(withoutPrefix);
			if (info.hand == ITrackedDevice::HAND_NONE) {
				OOVR_LOGF("WARNING: Ignoring invalid pose binding '%s' for %s, bad hand prefix", specPath.c_str(), action->fullName.c_str());
				continue;
			}

			// Parse the paths, matching them up with the components they correspond to
			if (withoutPrefix == "pose/raw" || withoutPrefix == "pose/aim") {
				info.point = PoseBindingPoint::RAW;
			} else if (withoutPrefix == "pose/grip" || withoutPrefix == "pose/handgrip") {
				// Not sure if /grip is valid or not, but it was there previously so we'll continue with it...
				info.point = PoseBindingPoint::HANDGRIP;
			} else if (withoutPrefix == "pose/base") {
				info.point = PoseBindingPoint::BASE;
			} else if (withoutPrefix == "pose/tip") {
				info.point = PoseBindingPoint::TIP;
			} else if (withoutPrefix == "pose/body") {
				info.point = PoseBindingPoint::BODY;
			} else if (withoutPrefix == "pose/gdc2015") {
				info.point = PoseBindingPoint::GDC2015;
			} else {
				OOVR_LOGF("WARNING: Ignoring unknown pose path '%s' (%s) for action %s", specPath.c_str(), withoutPrefix.c_str(), action->fullName.c_str());
				continue;
			}

			// We only accommodate one binding per hand, since having multiple doesn't make sense - either
			// a hand is tracked or not, since we derive everything from the grip pose.
			if (info.hand == ITrackedDevice::HAND_LEFT) {
				action->poseBindingsLeft[&profile] = info;
			} else {
				action->poseBindingsRight[&profile] = info;
			}
		}

		for (const auto& item : setJson["haptics"]) {
			std::string pathStr = lowerStr(item["path"].asString());

			std::string actionName = lowerStr(item["output"].asString());
			Action* action = actions.LookupItem(actionName);
			if (action == nullptr)
				OOVR_ABORTF("Missing haptic action '%s' in bindings file '%s'", actionName.c_str(), bindingsPath.c_str());

			if (!profile.IsInputPathValid(pathStr)) {
				OOVR_ABORTF("Built invalid input path %s from pose action %s", pathStr.c_str(), actionName.c_str());
			}

			XrPath path;
			OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, pathStr.c_str(), &path));
			bindings.push_back(XrActionSuggestedBinding{ action->xr, path });
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

void BaseInput::LoadDpadAction(const InteractionProfile& profile, const std::string& importBasePath, const std::string& inputName, const std::string& subMode, Action* action, std::vector<XrActionSuggestedBinding>& bindings)
{
	// special case for dpad: we need to create additional inputs and read them ourselves
	// verify that we actually have an input that can be used as an dpad for this profile
	std::string parentPath = profile.TranslateAction(importBasePath);
	if (!profile.IsInputPathValid(parentPath)) {
		OOVR_LOGF("WARNING: No such input path %s for profile %s, cannot bind dpad inputs, skipping", parentPath.c_str(), profile.GetPath().c_str());
		return;
	}

	// check direction
	auto dir_iter = DpadBindingInfo::directionMap.find(inputName);
	if (dir_iter == DpadBindingInfo::directionMap.end()) {
		OOVR_LOGF("WARNING: Unknown dpad direction %s given, skipping binding", inputName.c_str());
		return;
	}
	DpadBindingInfo dpad_info;
	dpad_info.direction = dir_iter->second;

	if (subMode != "touch" && subMode != "click") {
		OOVR_LOGF("WARNING: Unknown dpad sub mode %s given, skipping binding", subMode.c_str());
		return;
	}

	// check if parent is in dpadBindingParens
	// get parent name: remove /user/hand and /input/ parts, add end of openxr path (so we don't i.e. confuse dpad bindings from the knuckles joystick with dpad bindings from the oculus joystick)
	std::string to_delete[] = { "/user/hand/", "/input/" };
	auto& path = profile.GetPath();
	std::string end = path.substr(path.rfind("/") + 1);
	std::string parentName = importBasePath + "-" + end + "-dpad-parent";
	for (const auto& str : to_delete) {
		parentName.replace(parentName.find(str), str.size(), "");
	}

	XrActionCreateInfo info{ XR_TYPE_ACTION_CREATE_INFO };
	// dpads can be on either hand: need to set subaction paths for GetAnalogActionData
	info.subactionPaths = allSubactionPaths.data();
	info.countSubactionPaths = allSubactionPaths.size();

	auto parent_iter = DpadBindingInfo::parents.find(parentName);
	if (parent_iter == DpadBindingInfo::parents.end()) {
		// create mapping
		DpadBindingInfo::parents.insert({ parentName, DpadBindingInfo::ParentActions{} });
		parent_iter = DpadBindingInfo::parents.find(parentName);

		// create action for getting parent data (i.e. trackpad location)
		strcpy_arr(info.actionName, parentName.c_str());
		info.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
		strcpy_arr(info.localizedActionName, parentName.c_str()); // TODO localization
		OOVR_FAILED_XR_ABORT(xrCreateAction(action->set->xr, &info, &parent_iter->second.vectorAction));

		// add parent to bindings
		XrPath suggested_path;
		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, profile.TranslateAction(importBasePath).c_str(), &suggested_path));
		bindings.push_back(XrActionSuggestedBinding{ parent_iter->second.vectorAction, suggested_path });
	}

	if (subMode == "click") {
		dpad_info.click = true;
		if (parent_iter->second.clickAction == XR_NULL_HANDLE) {
			std::string click_name = parentName + "-click";
			strcpy_arr(info.actionName, click_name.c_str());
			info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
			strcpy_arr(info.localizedActionName, click_name.c_str());
			OOVR_FAILED_XR_ABORT(xrCreateAction(action->set->xr, &info, &parent_iter->second.clickAction));

			XrPath suggested_path;
			OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, profile.TranslateAction(importBasePath + "/click").c_str(), &suggested_path));
			bindings.push_back(XrActionSuggestedBinding{ parent_iter->second.clickAction, suggested_path });
		}
	} else {
		// touch dpad
		dpad_info.click = false;
		std::string touchPath = profile.TranslateAction(importBasePath + "/touch");
		if (!profile.IsInputPathValid(touchPath)) {
			OOVR_LOGF("WARNING: Path %s does not exist for the touch dpad.", touchPath.c_str());
		} else if (parent_iter->second.touchAction == XR_NULL_HANDLE) {
			std::string touch_name = parentName + "-touch";
			strcpy_arr(info.actionName, touch_name.c_str());
			info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
			strcpy_arr(info.localizedActionName, touch_name.c_str());
			OOVR_FAILED_XR_ABORT(xrCreateAction(action->set->xr, &info, &parent_iter->second.touchAction));

			XrPath suggested_path;
			OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, touchPath.c_str(), &suggested_path));
			bindings.push_back(XrActionSuggestedBinding{ parent_iter->second.touchAction, suggested_path });
		}
	}

	// add dpad parent to action
	action->dpadBindings.push_back({ parentName, dpad_info });
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
		ctrl.handType = i == 0 ? ITrackedDevice::HAND_LEFT : ITrackedDevice::HAND_RIGHT;
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
		create(&ctrl.gripClick, "grip-click", "Grip (Digital)", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.trigger, "trigger", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT);
		create(&ctrl.triggerTouch, "trigger-touch", "Trigger (Touch)", XR_ACTION_TYPE_BOOLEAN_INPUT);
		create(&ctrl.triggerClick, "trigger-click", "Trigger (Digital)", XR_ACTION_TYPE_BOOLEAN_INPUT);

		create(&ctrl.haptic, "haptic", "Vibration Haptics", XR_ACTION_TYPE_VIBRATION_OUTPUT);

		create(&ctrl.gripPoseAction, "grip-pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT);
		create(&ctrl.aimPoseAction, "aim-pose", "Aim Pose", XR_ACTION_TYPE_POSE_INPUT);
	}
}

EVRInputError BaseInput::GetActionSetHandle(const char* pchActionSetName, VRActionSetHandle_t* pHandle)
{
	*pHandle = actionSets.LookupHandle(pchActionSetName);
	return vr::VRInputError_None;
}

EVRInputError BaseInput::GetActionHandle(const char* pchActionName, VRActionHandle_t* pHandle)
{
	*pHandle = actions.LookupHandle(pchActionName);
	return vr::VRInputError_None;
}

EVRInputError BaseInput::GetInputSourceHandle(const char* pchInputSourcePath, VRInputValueHandle_t* pHandle)
{
	// Get the existing InputValueHandle if it already exists, or make a new one otherwise. Applications can
	// get whatever handles they want, regardless of whether the runtime associates any special meaning with it.

	const auto iter = inputHandleRegistry.find(pchInputSourcePath);
	if (iter != inputHandleRegistry.end()) {
		InputValueHandle* handle = iter->second.get();
		*pHandle = (uint64_t)handle;
		return VRInputError_None;
	}

	std::unique_ptr<InputValueHandle> handle = std::make_unique<InputValueHandle>();
	InputValueHandle* ptr = handle.get();
	handle->path = pchInputSourcePath;

	// Yes, this will let through something like/user/hand/leftblah but it's probably not an issue
	static std::string leftHand = "/user/hand/left";
	static std::string rightHand = "/user/hand/right";
	if (strncmp(pchInputSourcePath, leftHand.c_str(), leftHand.size()) == 0) {
		handle->type = InputSource::HAND_LEFT;
		handle->devicePathString = leftHand;
	}
	if (strncmp(pchInputSourcePath, rightHand.c_str(), rightHand.size()) == 0) {
		handle->type = InputSource::HAND_RIGHT;
		handle->devicePathString = rightHand;
	}

	if (!handle->devicePathString.empty()) {
		OOVR_FAILED_XR_ABORT(xrStringToPath(xr_instance, handle->devicePathString.c_str(), &handle->devicePath));

		// Very much should always be true, but guard against any nasty surprises
		OOVR_FALSE_ABORT(std::count(allSubactionPaths.begin(), allSubactionPaths.end(), handle->devicePath));
	}

	inputHandleRegistry[pchInputSourcePath] = std::move(handle);
	*(uint64_t*)pHandle = (uint64_t)ptr;
	return VRInputError_None;
}

EVRInputError BaseInput::UpdateActionState(VR_ARRAY_COUNT(unSetCount) VRActiveActionSet_t* pSets,
    uint32_t unSizeOfVRSelectedActionSet_t, uint32_t unSetCount)
{
	// TODO if the game is using legacy input, call this every frame

	OOVR_FALSE_ABORT(sizeof(*pSets) == unSizeOfVRSelectedActionSet_t);

	// Make sure all the ActionSets have the same priority, since we don't have any way around that right now
	if (unSetCount > 1) {
		int priority = pSets[0].nPriority;
		for (int i = 1; i < unSetCount; i++) {
			if (pSets[i].nPriority != priority) {
				ActionSet* as1 = cast_ASH(pSets[0].ulActionSet);
				ActionSet* curAs = cast_ASH(pSets[1].ulActionSet);
				OOVR_SOFT_ABORTF("Active action set %s (%d) and %s (%d) have different priorities, this is not yet supported",
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
			ITrackedDevice* dev = ivhToDev(set.ulRestrictedToDevice);
			if (dev && dev->GetHand() != ITrackedDevice::HAND_NONE) {
				LegacyControllerActions& ctrl = legacyControllers[dev->GetHand()];
				aas[i].subactionPath = ctrl.handPathXr;
			}
		}
	}

	// Ad the last set, the legacy input set
	aas.at(unSetCount).actionSet = legacyInputsSet;

	XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
	syncInfo.activeActionSets = aas.data();
	syncInfo.countActiveActionSets = aas.size();
	OOVR_FAILED_XR_ABORT(xrSyncActions(xr_session.get(), &syncInfo));
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
	OOVR_FAILED_XR_ABORT(xrSyncActions(xr_session.get(), &syncInfo));
	syncSerial++;
}

XrResult BaseInput::getBooleanOrDpadData(Action& action, const XrActionStateGetInfo* getInfo, XrActionStateBoolean* state)
{
	*state = { XR_TYPE_ACTION_STATE_BOOLEAN };

	// If an action is bound to a dpad action in every profile, action.xr will be XR_NULL_HANDLE
	if (action.xr != XR_NULL_HANDLE) {
		XrResult ret = xrGetActionStateBoolean(xr_session.get(), getInfo, state);
		OOVR_FAILED_XR_ABORT(ret);

		// actions could be bound to regular buttons and dpad buttons
		if (action.dpadBindings.empty() || state->currentState == XR_TRUE)
			return ret;
	}

	// dpad bindings: need to read parent state(s) and fill in state ourselves
	for (auto& [parent_name, dpad_info] : action.dpadBindings) {
		// if we've already determined one of the bindings is active no need to continue
		if (state->currentState == XR_TRUE)
			break;
		// read state of parent
		XrActionStateVector2f parent_state = { XR_TYPE_ACTION_STATE_VECTOR2F };
		auto iter = DpadBindingInfo::parents.find(parent_name);
		OOVR_FALSE_ABORT(iter != DpadBindingInfo::parents.end());
		XrActionStateGetInfo info2 = *getInfo;
		info2.action = iter->second.vectorAction;
		OOVR_FAILED_XR_ABORT(xrGetActionStateVector2f(xr_session.get(), &info2, &parent_state));

		// convert to polar coordinates
		// angle is in radians
		float radius = sqrt(pow(parent_state.currentState.x, 2) + pow(parent_state.currentState.y, 2));
		float angle = atan2(parent_state.currentState.y, parent_state.currentState.x);

		// note: since the range of atan2 is (-pi, pi), the east and west directions will have points between the negative and positive portions (if you think of the dpad as a circle)
		bool within_bounds = false;
		switch (dpad_info.direction) {
		case DpadBindingInfo::Direction::CENTER: {
			within_bounds = (radius <= DpadBindingInfo::dpadDeadzoneRadius);
			break;
		}
		case DpadBindingInfo::Direction::NORTH: {
			within_bounds = (radius > DpadBindingInfo::dpadDeadzoneRadius) && (angle > DpadBindingInfo::angle45deg && angle <= DpadBindingInfo::angle135deg);
			break;
		}
		case DpadBindingInfo::Direction::WEST: {
			within_bounds = (radius > DpadBindingInfo::dpadDeadzoneRadius) && (angle > DpadBindingInfo::angle135deg || angle <= -DpadBindingInfo::angle135deg);
			break;
		}
		case DpadBindingInfo::Direction::SOUTH: {
			within_bounds = (radius > DpadBindingInfo::dpadDeadzoneRadius) && (angle > -DpadBindingInfo::angle135deg && angle <= -DpadBindingInfo::angle45deg);
			break;
		}
		case DpadBindingInfo::Direction::EAST: {
			within_bounds = (radius > DpadBindingInfo::dpadDeadzoneRadius) && (angle > -DpadBindingInfo::angle45deg && angle <= DpadBindingInfo::angle45deg);
			break;
		}
		}

		bool active;
		if (dpad_info.click) {
			XrActionStateBoolean click_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
			info2.action = iter->second.clickAction;
			OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session.get(), &info2, &click_state));
			active = click_state.currentState;
		} else if (iter->second.touchAction != XR_NULL_HANDLE) {
			XrActionStateBoolean touch_state{ XR_TYPE_ACTION_STATE_BOOLEAN };
			info2.action = iter->second.touchAction;
			OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session.get(), &info2, &touch_state));
			active = touch_state.currentState;
		} else {
			// touch dpad, but our dpad parent doesn't have a touch input
			active = true;
		}

		if (active && within_bounds) {
			state->currentState = XR_TRUE;
		} else {
			state->currentState = XR_FALSE;
		}

		if (state->currentState != dpad_info.lastState) {
			state->changedSinceLastSync = XR_TRUE;
			state->lastChangeTime = parent_state.lastChangeTime;
			dpad_info.lastState = state->currentState;
		} else {
			state->changedSinceLastSync = XR_FALSE;
		}
		state->isActive = parent_state.isActive;
	}
	return XR_SUCCESS;
}

EVRInputError BaseInput::GetDigitalActionData(VRActionHandle_t action, InputDigitalActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	GET_ACTION_FROM_HANDLE(act, action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = act->xr;

	// Unfortunately to implement activeOrigin we have to loop through and query each action state
	for (int i = 0; i < allSubactionPaths.size(); i++) {
		XrPath subactionPath = allSubactionPaths[i];

		if (!checkRestrictToDevice(ulRestrictToDevice, subactionPath))
			continue;

		getInfo.subactionPath = subactionPath;
		XrActionStateBoolean state = { XR_TYPE_ACTION_STATE_BOOLEAN };
		OOVR_FAILED_XR_ABORT(getBooleanOrDpadData(*act, &getInfo, &state));

		// If the subaction isn't set, or it was set but not active, or it was set
		// but the state was false and it's not now, then override it.
		if (!(state.isActive > pActionData->bActive || state.currentState > pActionData->bState))
			continue;

		pActionData->bState = state.currentState;
		pActionData->bActive = state.isActive;
		pActionData->bChanged = state.changedSinceLastSync;
		// TODO implement fUpdateTime
		pActionData->activeOrigin = activeOriginFromSubaction(act, allSubactionPathNames[i].c_str());
	}

	// Note it's possible we didn't set any output if this action isn't bound to anything, just leave the
	//  struct at it's default values.

	return VRInputError_None;
}

EVRInputError BaseInput::GetAnalogActionData(VRActionHandle_t action, InputAnalogActionData_t* pActionData, uint32_t unActionDataSize,
    VRInputValueHandle_t ulRestrictToDevice)
{
	GET_ACTION_FROM_HANDLE(act, action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = act->xr;

	// Only return the input with the greatest magnitude
	// To do this, track the input with the greatest length.
	float maxLengthSq = 0;

	// Unfortunately to implement activeOrigin we have to loop through and query each action state
	for (int i = 0; i < allSubactionPaths.size(); i++) {
		XrPath subactionPath = allSubactionPaths[i];
		if (!checkRestrictToDevice(ulRestrictToDevice, subactionPath))
			continue;

		getInfo.subactionPath = subactionPath;

		switch (act->type) {
		case ActionType::Vector1: {
			XrActionStateFloat state = { XR_TYPE_ACTION_STATE_FLOAT };
			OOVR_FAILED_XR_ABORT(xrGetActionStateFloat(xr_session.get(), &getInfo, &state));

			float lengthSq = state.currentState * state.currentState;
			if (lengthSq < maxLengthSq || !state.isActive)
				continue;
			lengthSq = maxLengthSq;

			pActionData->x = state.currentState;
			pActionData->y = 0;
			pActionData->z = 0;
			pActionData->deltaX = state.currentState - act->previousState.x;
			pActionData->bActive = state.isActive;
			pActionData->activeOrigin = activeOriginFromSubaction(act, allSubactionPathNames[i].c_str());

			act->previousState.x = state.currentState;
			break;
		}
		case ActionType::Vector2: {
			XrActionStateVector2f state = { XR_TYPE_ACTION_STATE_VECTOR2F };
			OOVR_FAILED_XR_ABORT(xrGetActionStateVector2f(xr_session.get(), &getInfo, &state));

			float lengthSq = state.currentState.x * state.currentState.x + state.currentState.y * state.currentState.y;
			if (lengthSq < maxLengthSq || !state.isActive)
				continue;
			maxLengthSq = lengthSq;

			pActionData->x = state.currentState.x;
			pActionData->y = state.currentState.y;
			pActionData->z = 0;
			pActionData->deltaX = state.currentState.x - act->previousState.x;
			pActionData->deltaY = state.currentState.y - act->previousState.y;
			pActionData->bActive = state.isActive;
			pActionData->activeOrigin = activeOriginFromSubaction(act, allSubactionPathNames[i].c_str());

			act->previousState.x = state.currentState.x;
			act->previousState.y = state.currentState.y;
			break;
		}
		case ActionType::Vector3:
			OOVR_ABORTF("Input type vector3 unsupported: %s", act->fullName.c_str());
			break;
		default:
			OOVR_ABORTF("Invalid action type %d for action %s", act->type, act->fullName.c_str());
			break;
		}
	}

	// TODO implement the deltas

	return VRInputError_None;
}

EVRInputError BaseInput::GetPoseActionData(VRActionHandle_t action, ETrackingUniverseOrigin eOrigin, float fPredictedSecondsFromNow,
    InputPoseActionData_t* pActionData, uint32_t unActionDataSize, VRInputValueHandle_t ulRestrictToDevice)
{
	GET_ACTION_FROM_HANDLE(act, action);

	ZeroMemory(pActionData, unActionDataSize);
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*pActionData));

	// Skeletons go through the legacy input thing, since they're tightly bound to either hand
	if (act->type == ActionType::Skeleton) {
		XrSpace space = legacyControllers[act->skeletalHand].gripPoseSpace;

		pActionData->bActive = true; // TODO this should probably come from reading the skeleton data
		pActionData->activeOrigin = vr::k_ulInvalidInputValueHandle; // TODO implement activeOrigin
		xr_utils::PoseFromSpace(&pActionData->pose, space, eOrigin);

		return vr::VRInputError_None;
	}

	if (act->type != ActionType::Pose)
		OOVR_ABORTF("Invalid action type %d for action %s", act->type, act->fullName.c_str());

	const InputValueHandle* restrictToDevice = nullptr;
	if (ulRestrictToDevice)
		restrictToDevice = cast_IVH(ulRestrictToDevice);

	for (int handNum = 0; handNum < 2; handNum++) {
		// Check this hand is permitted
		ITrackedDevice::HandType handType = handNum == 0 ? ITrackedDevice::HAND_LEFT : ITrackedDevice::HAND_RIGHT;
		InputSource sourceType = handNum == 0 ? InputSource::HAND_LEFT : InputSource::HAND_RIGHT;

		if (restrictToDevice && restrictToDevice->type != sourceType) {
			continue;
		}

		// Find the device for this hand, and look up it's interaction profile
		ITrackedDevice* dev = BackendManager::Instance().GetDeviceByHand(handType);
		if (!dev)
			continue;

		const InteractionProfile* profile = dev->GetInteractionProfile();
		if (!profile)
			continue;

		const auto& bindings = handType == ITrackedDevice::HAND_LEFT ? act->poseBindingsLeft : act->poseBindingsRight;
		const auto bindingIter = bindings.find(profile);
		if (bindingIter == bindings.end())
			continue;
		const PoseBindingInfo& binding = bindingIter->second;

		// TODO use predicted time

		// Regardless of whether valid data is available, this action is bound
		pActionData->bActive = true;

		// TODO make sure this is correct
		pActionData->activeOrigin = activeOriginFromSubaction(act, allSubactionPathNames[handNum].c_str());

		vr::TrackedDevicePose_t rawPose = {};
		dev->GetPose(eOrigin, &rawPose, TrackingStateType_Now);

		if (rawPose.bPoseIsValid) {
			glm::mat4 handMat = S2G_m34(rawPose.mDeviceToAbsoluteTracking);

			const char* componentName = nullptr;
			switch (binding.point) {
			case PoseBindingPoint::RAW:
				// No component name
				break;
			case PoseBindingPoint::BASE:
				componentName = "base";
				break;
			case PoseBindingPoint::HANDGRIP:
				componentName = "handgrip";
				break;
			case PoseBindingPoint::TIP:
				componentName = "tip";
				break;
			}

			// Add the component transform, if required
			OOVR_RenderModel_ComponentState_t state = {};
			bool hasTransform = false;
			glm::mat4 transform = glm::identity<glm::mat4>();
			if (componentName) {
				hasTransform = GetBaseRenderModels()->TryGetComponentState(handType, componentName, &state);
			}
			if (hasTransform) {
				transform = S2G_m34(state.mTrackingToComponentLocal);

				handMat = handMat * transform;
			}

			// Store the final hand matrix
			pActionData->pose.mDeviceToAbsoluteTracking = G2S_m34(handMat);
			pActionData->pose.bPoseIsValid = true;
			pActionData->pose.bDeviceIsConnected = true;
			pActionData->pose.eTrackingResult = rawPose.eTrackingResult;

			// The velocity stays in the original coordinate system, no translation required
			pActionData->pose.vVelocity = rawPose.vVelocity;

			// TODO what about angular velocity?
			pActionData->pose.vAngularVelocity = rawPose.vAngularVelocity;
		}

		// TODO implement the deltas

		// Stop as soon as we find the first available input (and a disconnected controller is
		// fine to count as available - that's probably not how it should be, but it'd be a
		// pain to do it properly)
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
	// This is the old version of the function, the new one doesn't have the device restriction
	// Hopefully noone depends on this and we can just ignore it
	if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle) {
		OOVR_SOFT_ABORT("Old GetSkeletalActionData device restrictions not supported");
	}

	return GetSkeletalActionData(action, pActionData, unActionDataSize);
}
EVRInputError BaseInput::GetSkeletalActionData(VRActionHandle_t actionHandle, InputSkeletalActionData_t* out, uint32_t unActionDataSize)
{
	// Make sure the target struct is the right size, in case it grows in the future
	// Note: CVRInput_004 has a manual override for this in CVRInput.cpp to deal with an old struct version, it
	// sets unActionDataSize to what we're expecting so we don't have to handle that here.
	OOVR_FALSE_ABORT(unActionDataSize == sizeof(*out));

	GET_ACTION_FROM_HANDLE(action, actionHandle);

	// Zero out the output data, leaving:
	// bActive = false
	// activeOrigin = vr::k_ulInvalidActionHandle
	ZeroMemory(out, unActionDataSize);

	// If there's no action, say there's nothing on it
	if (action == nullptr) {
		return vr::VRInputError_None;
	}

	// Same for if the runtime doesn't support hand-tracking
	if (!xr_gbl->handTrackingProperties.supportsHandTracking) {
		OOVR_SOFT_ABORT("Runtime does not support hand-tracking, skeletal data unavailable");
		return vr::VRInputError_None;
	}

	// Find the active origin for this hand
	std::string originPath;
	if (action->skeletalHand == ITrackedDevice::HAND_LEFT)
		originPath = "/user/hand/left";
	else
		originPath = "/user/hand/right";
	OOVR_FALSE_ABORT(GetInputSourceHandle(originPath.c_str(), &out->activeOrigin) == vr::VRInputError_None);

	out->bActive = true; // TODO run xrLocateHandJointsEXT and use it's isActive
	return vr::VRInputError_None;
}
EVRInputError BaseInput::GetDominantHand(vr::ETrackedControllerRole* peDominantHand)
{
	// The API documentation says we need allowSetDominantHand for this, but that
	// seems like a mistake (i.e. that comment was probably meant to be on
	// SetDominantHand), so I'll just allow it always.
	*peDominantHand = dominantHand;
	return VRInputError_None;
}
EVRInputError BaseInput::SetDominantHand(vr::ETrackedControllerRole eDominantHand)
{
	if (allowSetDominantHand) {
		dominantHand = eDominantHand;
		return VRInputError_None;
	} else {
		return VRInputError_PermissionDenied;
	}
}
EVRInputError BaseInput::GetBoneCount(VRActionHandle_t action, uint32_t* pBoneCount)
{
	// Maybe we should check the action?
	*pBoneCount = (int)eBone_Count;
	return vr::VRInputError_None;
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
	OOVR_SOFT_ABORT("Skeletal reference transforms not implemented");
	return vr::VRInputError_InvalidSkeleton;
}
EVRInputError BaseInput::GetSkeletalTrackingLevel(VRActionHandle_t action, EVRSkeletalTrackingLevel* pSkeletalTrackingLevel)
{
	if (xr_gbl->handTrackingProperties.supportsHandTracking) {
		// We can't know if this should be partial or full, but I'm guessing
		// applications don't care too much in practice
		*pSkeletalTrackingLevel = vr::VRSkeletalTracking_Full;
	} else {
		*pSkeletalTrackingLevel = vr::VRSkeletalTracking_Estimated;
	}

	return vr::VRInputError_None;
}
EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t action, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray,
    uint32_t unTransformArrayCount, VRInputValueHandle_t ulRestrictToDevice)
{
	// This is the old version of the function, the new one doesn't have the device restriction
	// Hopefully noone depends on this and we can just ignore it
	if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle) {
		OOVR_SOFT_ABORT("Old skeletal input device restrictions not supported");
	}

	return GetSkeletalBoneData(action, eTransformSpace, eMotionRange, pTransformArray, unTransformArrayCount);
}

EVRInputError BaseInput::GetSkeletalBoneData(VRActionHandle_t actionHandle, EVRSkeletalTransformSpace eTransformSpace,
    EVRSkeletalMotionRange eMotionRange, VR_ARRAY_COUNT(unTransformArrayCount) VRBoneTransform_t* pTransformArray, uint32_t unTransformArrayCount)
{
	ZeroMemory(pTransformArray, sizeof(VRBoneTransform_t) * unTransformArrayCount);
	GET_ACTION_FROM_HANDLE(action, actionHandle);

	// FIXME remove
	// Hand labs is in parent transform space, and usually 'with controller' motion range
	// OOVR_LOGF("skeletal data %d %d", eTransformSpace, eMotionRange);

	// Check for the right number of bones
	OOVR_FALSE_ABORT(unTransformArrayCount == 31);

	// If there's no action, leave the transforms zeroed out
	if (action == nullptr) {
		return vr::VRInputError_None;
	}

	// If the runtime doesn't support hand-tracking, leave the data zeroed out. We should eventually
	// make our own data in this case.
	if (!xr_gbl->handTrackingProperties.supportsHandTracking) {
		OOVR_SOFT_ABORT("Runtime does not support hand-tracking, skeletal data unavailable");
		return vr::VRInputError_None;
	}

	XrHandJointsLocateInfoEXT locateInfo = { XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
	locateInfo.baseSpace = legacyControllers[(int)action->skeletalHand].aimPoseSpace;
	locateInfo.time = xr_gbl->GetBestTime();

	XrHandJointLocationsEXT locations = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
	locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
	std::vector<XrHandJointLocationEXT> jointLocations(locations.jointCount);
	locations.jointLocations = jointLocations.data();

	OOVR_FAILED_XR_ABORT(xr_ext->xrLocateHandJointsEXT(handTrackers[(int)action->skeletalHand], &locateInfo, &locations));

	if (!locations.isActive) {
		// Leave empty-handed, IDK if this is the right error or not
		return vr::VRInputError_InvalidSkeleton;
	}

	bool isRight = (action->skeletalHand == ITrackedDevice::HAND_RIGHT);

	if (eTransformSpace == VRSkeletalTransformSpace_Model) {
		ConvertHandModelSpace(jointLocations, isRight, pTransformArray);
	} else {
		ConvertHandParentSpace(jointLocations, isRight, pTransformArray);
	}

	// For now, just return with non-active data
	return vr::VRInputError_None;
}
EVRInputError BaseInput::GetSkeletalSummaryData(VRActionHandle_t actionHandle, EVRSummaryType eSummaryType, VRSkeletalSummaryData_t* pSkeletalSummaryData)
{
	GET_ACTION_FROM_HANDLE(action, actionHandle);

	if (action == nullptr) {
		return vr::VRInputError_None;
	}

	if (!xr_gbl->handTrackingProperties.supportsHandTracking) {
		// TODO: generate our own data as mentioned above. We might want to use the
		// generated summary data to generate the bone data.
		OOVR_SOFT_ABORT("Runtime does not support hand-tracking, skeletal summary data unavailable");
		return vr::VRInputError_None;
	}

	XrHandJointsLocateInfoEXT locateInfo = { XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
	locateInfo.baseSpace = legacyControllers[(int)action->skeletalHand].aimPoseSpace;
	locateInfo.time = xr_gbl->GetBestTime();

	XrHandJointLocationsEXT locations = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
	locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
	std::vector<XrHandJointLocationEXT> jointLocations(locations.jointCount);
	locations.jointLocations = jointLocations.data();

	OOVR_FAILED_XR_ABORT(xr_ext->xrLocateHandJointsEXT(handTrackers[(int)action->skeletalHand], &locateInfo, &locations));

	if (!locations.isActive) {
		// Leave empty-handed, IDK if this is the right error or not
		return vr::VRInputError_InvalidSkeleton;
	}

	for (int i = 0; i < 5; ++i) {
		XrHandJointLocationEXT metacarpal, proximal, tip;

		switch (i) {
		case 0: // thumb
			metacarpal = jointLocations[XR_HAND_JOINT_THUMB_METACARPAL_EXT];
			proximal = jointLocations[XR_HAND_JOINT_THUMB_PROXIMAL_EXT];
			tip = jointLocations[XR_HAND_JOINT_THUMB_TIP_EXT];
			break;
		case 1: // index
			metacarpal = jointLocations[XR_HAND_JOINT_INDEX_METACARPAL_EXT];
			proximal = jointLocations[XR_HAND_JOINT_INDEX_PROXIMAL_EXT];
			tip = jointLocations[XR_HAND_JOINT_INDEX_TIP_EXT];
			break;
		case 2: // middle
			metacarpal = jointLocations[XR_HAND_JOINT_MIDDLE_METACARPAL_EXT];
			proximal = jointLocations[XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT];
			tip = jointLocations[XR_HAND_JOINT_MIDDLE_TIP_EXT];
			break;
		case 3: // ring
			metacarpal = jointLocations[XR_HAND_JOINT_RING_METACARPAL_EXT];
			proximal = jointLocations[XR_HAND_JOINT_RING_PROXIMAL_EXT];
			tip = jointLocations[XR_HAND_JOINT_RING_TIP_EXT];
			break;
		case 4: // little
			metacarpal = jointLocations[XR_HAND_JOINT_LITTLE_METACARPAL_EXT];
			proximal = jointLocations[XR_HAND_JOINT_LITTLE_PROXIMAL_EXT];
			tip = jointLocations[XR_HAND_JOINT_LITTLE_TIP_EXT];
			break;
		default:
			break;
		}

		glm::vec3 metacarpalProximalDelta = X2G_v3f(metacarpal.pose.position) - X2G_v3f(proximal.pose.position);
		glm::vec3 tipProximalDelta = X2G_v3f(tip.pose.position) - X2G_v3f(proximal.pose.position);

		float dot = glm::dot(metacarpalProximalDelta, tipProximalDelta);
		float a = glm::length(metacarpalProximalDelta);
		float b = glm::length(tipProximalDelta);
		// dot = a * b * cos(ang)

		float curl;
		if (a == 0.0f || b == 0.0f) {
			// That's probably not meant to happen? But if two joints really do
			// coincide then that probably means the hand is curled up
			curl = 1.0f;
		} else {
			// Find the angle between these three joints
			float angCos = dot / (a * b);
			if (angCos < -1.0f)
				angCos = -1.0f;
			if (angCos > 1.0f)
				angCos = 1.0f;
			float ang = acosf(angCos);
			curl = 1.0f - (ang / M_PI);
		}

		pSkeletalSummaryData->flFingerCurl[i] = curl;
	}

	for (int i = 0; i < 4; ++i) {
		OOVR_SOFT_ABORT("Finger splay hardcoded at 0.2");
		pSkeletalSummaryData->flFingerSplay[i] = 0.2f; // TODO
	}

	return vr::VRInputError_None;
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
	GET_ACTION_FROM_HANDLE(act, action);

	if (act->type != ActionType::Vibration) {
		OOVR_ABORTF("Cannot trigger vibration on non-vibration (type=%d) action '%s'", act->type, act->fullName.c_str());
	}

	// TODO check the subaction stuff works properly
	XrPath subactionPath = XR_NULL_PATH;
	if (ulRestrictToDevice != vr::k_ulInvalidInputValueHandle) {
		ITrackedDevice* dev = ivhToDev(ulRestrictToDevice);
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
	vibration.frequency = fFrequency == 0.0f ? XR_FREQUENCY_UNSPECIFIED : fFrequency;
	vibration.duration = (int)(fDurationSeconds * 1000000000.0f);
	vibration.amplitude = fAmplitude;

	OOVR_FAILED_XR_ABORT(xrApplyHapticFeedback(xr_session.get(), &info, (XrHapticBaseHeader*)&vibration));

	return VRInputError_None;
}

EVRInputError BaseInput::GetActionOrigins(VRActionSetHandle_t actionSetHandle, VRActionHandle_t digitalActionHandle,
    VR_ARRAY_COUNT(originOutCount) VRInputValueHandle_t* originsOut, uint32_t originOutCount)
{
	GET_ACTION_SET_FROM_HANDLE(set, actionSetHandle);
	GET_ACTION_FROM_HANDLE(act, digitalActionHandle);

	// TODO find something that passes in non-matching values and see what results it wants, or try it with SteamVR
	if (act->set != set) {
		OOVR_ABORTF("GetActionOrigins: set mismatch %s vs %s", set->name.c_str(), act->fullName.c_str());
	}

	if (act->type == ActionType::Skeleton || act->type == ActionType::Pose) {
		// TODO what should this do?
		OOVR_SOFT_ABORT("Skeleton and pose action origins not implemented, returning error");
		return VRInputError_InvalidHandle;
	}

	ZeroMemory(originsOut, originOutCount * sizeof(*originsOut));

	std::set<std::string> sources;
	XrBoundSourcesForActionEnumerateInfo info = { XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO };
	info.action = act->xr;

	// 20 will be more than enough, saves a second call
	XrPath tmp[20];
	uint32_t count;
	OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session.get(), &info, std::size(tmp), &count, tmp));

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

	// Copy out the sources
	int i = 0;
	for (const std::string& path : sources) {
		if (i >= originOutCount)
			return vr::VRInputError_MaxCapacityReached; // TODO check this is correct
		OOVR_FALSE_ABORT(GetInputSourceHandle(path.c_str(), &originsOut[i]) == vr::VRInputError_None);
		i++;
	}

	return VRInputError_None;
}

EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize)
{
	return GetOriginLocalizedName(origin, pchNameArray, unNameArraySize, VRInputString_All);
}

EVRInputError BaseInput::GetOriginLocalizedName(VRInputValueHandle_t origin, VR_OUT_STRING() char* pchNameArray, uint32_t unNameArraySize,
    int32_t unStringSectionsToInclude)
{
	if (origin == vr::k_ulInvalidInputValueHandle)
		return vr::VRInputError_InvalidHandle;

	ITrackedDevice* dev = ivhToDev(origin);

	if (!dev)
		return VRInputError_InvalidHandle;

	if (!pchNameArray || unNameArraySize == 0)
		return VRInputError_MaxCapacityReached;

	std::string name;

	if (unStringSectionsToInclude & VRInputString_Hand) {
		switch (dev->GetHand()) {
		case ITrackedDevice::HAND_LEFT:
			name += "Left Hand ";
			break;
		case ITrackedDevice::HAND_RIGHT:
			name += "Right Hand ";
			break;
		default:
			name += "Unknown Hand ";
			break;
		}
	}

	if (unStringSectionsToInclude & VRInputString_ControllerType) {
		name += "OpenXR Controller ";
	}

	if (unStringSectionsToInclude & VRInputString_InputSource) {
		// TODO: what should go here?
		// name += "Something ";
	}

	if (name.size() > 0) {
		// Remove trailing space
		name.erase(name.size() - 1);
	}

	const char* str = name.c_str();

	size_t i;
	for (i = 0; str[i] && i < unNameArraySize - 1; ++i) {
		pchNameArray[i] = str[i];
	}
	pchNameArray[i] = 0;

	return str[i] ? VRInputError_MaxCapacityReached : VRInputError_None;
}

EVRInputError BaseInput::GetOriginTrackedDeviceInfo(VRInputValueHandle_t origin, InputOriginInfo_t* info, uint32_t unOriginInfoSize)
{
	memset(info, 0, unOriginInfoSize);
	OOVR_FALSE_ABORT(unOriginInfoSize == sizeof(InputOriginInfo_t));

	if (origin == vr::k_ulInvalidInputValueHandle)
		return vr::VRInputError_InvalidHandle;

	ITrackedDevice* dev = ivhToDev(origin);

	if (!dev)
		return vr::VRInputError_InvalidHandle;

	info->trackedDeviceIndex = dev->DeviceIndex();
	info->devicePath = origin;
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
	GET_ACTION_FROM_HANDLE(action, actionHandle);

	XrBoundSourcesForActionEnumerateInfo enumInfo = { XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO };
	enumInfo.action = action->xr;
	uint32_t sourcesCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session.get(), &enumInfo, 0, &sourcesCount, nullptr));
	std::vector<XrPath> boundActionPaths(sourcesCount);
	OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session.get(), &enumInfo, boundActionPaths.size(), &sourcesCount, boundActionPaths.data()));

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
	if (unVariantArraySize == 0) {
		return VRInputError_MaxCapacityReached;
	}

	OOVR_SOFT_ABORT("GetBindingVariant not implemented");

	*pchVariantArray = 0;
	return VRInputError_None;
}

BaseInput::Action* BaseInput::cast_AH(VRActionHandle_t handle)
{
	return actions.LookupItem((RegHandle)handle);
}

BaseInput::ActionSet* BaseInput::cast_ASH(VRActionSetHandle_t handle)
{
	return actionSets.LookupItem((RegHandle)handle);
}

BaseInput::InputValueHandle* BaseInput::cast_IVH(VRInputValueHandle_t handle)
{
	if (handle == vr::k_ulInvalidInputValueHandle)
		OOVR_ABORT("Called ivhToDev for invalid input value handle");

	return (InputValueHandle*)handle;
}

ITrackedDevice* BaseInput::ivhToDev(VRInputValueHandle_t handle)
{
	const InputValueHandle* ivh = cast_IVH(handle);

	ITrackedDevice::HandType hand = ITrackedDevice::HAND_NONE;
	switch (ivh->type) {
	case InputSource::HAND_LEFT:
		hand = ITrackedDevice::HAND_LEFT;
		break;
	case InputSource::HAND_RIGHT:
		hand = ITrackedDevice::HAND_RIGHT;
		break;
	default:
		OOVR_SOFT_ABORTF("Cannot get invalid device from handle '%s'", ivh->path.c_str());
		return nullptr;
	}

	return BackendManager::Instance().GetDeviceByHand(hand);
}

bool BaseInput::checkRestrictToDevice(vr::VRInputValueHandle_t restrictToDevice, XrPath subactionPath)
{
	if (restrictToDevice == vr::k_ulInvalidInputValueHandle)
		return true;

	const InputValueHandle* ivh = cast_IVH(restrictToDevice);
	return subactionPath == ivh->devicePath;
}

VRInputValueHandle_t BaseInput::activeOriginFromSubaction(Action* action, const char* subactionPath)
{
	// FIXME the docs for xrEnumerateBoundSourcesForAction are wrong and will be updated (source: rpavlik). They don't
	//  have to return a path listed in the input profile, they can be literally anything (not even a /user/hand/<side>
	//  prefix is guaranteed). Thus we'll need some sophisticated lying to the application about this.

	// If it's time for an update and this isn't a virtual input, update its sources
	if (syncSerial >= action->nextSourcesUpdate && action->xr) {
		// Get the attached sources
		XrBoundSourcesForActionEnumerateInfo enumInfo = { XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO };
		enumInfo.action = action->xr;
		OOVR_FAILED_XR_ABORT(xrEnumerateBoundSourcesForAction(xr_session.get(), &enumInfo,
		    std::size(action->sources), &action->sourcesCount, action->sources));

		// Convert the source paths to strings
		char buff[256];
		for (int i = 0; i < (int)action->sourcesCount; i++) {
			ZeroMemory(buff, sizeof(buff));
			uint32_t length;
			OOVR_FAILED_XR_ABORT(xrPathToString(xr_instance, action->sources[i], sizeof(buff) - 1, &length, buff));
			action->sourceNames[i] = buff;
		}

		// Wait 200 updates. This obviously depends on the framerate but probably only changes if a device is
		// disconnected or reconnected. It's unlikely it'll be an issue, but just in case randomise the timer
		// to prevent a thundering herd problem.
		action->nextSourcesUpdate = syncSerial + 200 + ((int)std::rand() % 100); // NOLINT(cert-msc50-cpp)
	}

	// Go through the input sources and find the first one that starts with the subaction path
	for (int i = 0; i < (int)action->sourcesCount; i++) {
		const std::string& str = action->sourceNames[i];
		if (strncmp(str.c_str(), subactionPath, strlen(subactionPath)) == 0) {
			VRInputValueHandle_t handle;
			OOVR_FALSE_ABORT(GetInputSourceHandle(str.c_str(), &handle) == vr::VRInputError_None);
			return handle;
		}
	}

	// Couldn't find one? Just use the subaction path as the device itself. Kinda ugly but it's unlikely to cause
	// an actual crash, and we'll soft-abort on it since this shouldn't actually happen, unless the user has just
	// connected a device and pressed a button before it had time to update.
	OOVR_SOFT_ABORTF("Couldn't find matching source for subaction path '%s' on action '%s'", subactionPath, action->fullName.c_str());
	VRInputValueHandle_t handle;
	OOVR_FALSE_ABORT(GetInputSourceHandle(subactionPath, &handle) == vr::VRInputError_None);
	return handle;
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
			OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session.get(), &getInfo, &xs));
			state->ulButtonPressed |= (uint64_t)(xs.currentState != 0) << shift;
		}

		if (touch != XR_NULL_HANDLE) {
			getInfo.action = touch;
			OOVR_FAILED_XR_ABORT(xrGetActionStateBoolean(xr_session.get(), &getInfo, &xs));
			state->ulButtonTouched |= (uint64_t)(xs.currentState != 0) << shift;
		}
	};

	// Read the buttons

	bindButton(ctrl.system, XR_NULL_HANDLE, vr::k_EButton_System);
	bindButton(ctrl.btnA, ctrl.btnATouch, vr::k_EButton_A);
	bindButton(ctrl.menu, ctrl.menuTouch, vr::k_EButton_ApplicationMenu);
	bindButton(ctrl.stickBtn, ctrl.stickBtnTouch, vr::k_EButton_SteamVR_Touchpad);
	bindButton(ctrl.gripClick, XR_NULL_HANDLE, vr::k_EButton_Grip);
	bindButton(ctrl.triggerClick, ctrl.triggerTouch, vr::k_EButton_SteamVR_Trigger);
	bindButton(XR_NULL_HANDLE, XR_NULL_HANDLE, vr::k_EButton_Axis2); // FIXME clean up? Is this the grip?

	// Read the analogue values
	auto readFloat = [](XrAction action) -> float {
		if (!action)
			return 0;

		XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
		getInfo.action = action;

		XrActionStateFloat as = { XR_TYPE_ACTION_STATE_FLOAT };
		OOVR_FAILED_XR_ABORT(xrGetActionStateFloat(xr_session.get(), &getInfo, &as));
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

	OOVR_FAILED_XR_ABORT(xrApplyHapticFeedback(xr_session.get(), &info, (XrHapticBaseHeader*)&vibration));
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

void BaseInput::GetHandSpace(vr::TrackedDeviceIndex_t index, XrSpace& space, bool aimPose)
{
	space = XR_NULL_HANDLE;

	// If the manifest isn't loaded yet (still on the first frame) return null
	if (!hasLoadedActions)
		return;

	ITrackedDevice* dev = BackendManager::Instance().GetDevice(index);
	if (!dev)
		return;

	ITrackedDevice::HandType hand = dev->GetHand();
	GetHandSpace(hand, space, aimPose);
}

void BaseInput::GetHandSpace(ITrackedDevice::HandType hand, XrSpace& space, bool aimPose)
{
	LegacyControllerActions& ctrl = legacyControllers[hand];

	space = aimPose ? ctrl.aimPoseSpace : ctrl.gripPoseSpace;
}

bool BaseInput::AreActionsLoaded()
{
	return hasLoadedActions;
}

ITrackedDevice::HandType BaseInput::ParseAndRemoveHandPrefix(std::string& toModify)
{
	static std::string leftPrefix = "/user/hand/left/";
	static std::string rightPrefix = "/user/hand/right/";

	if (toModify.starts_with(leftPrefix)) {
		toModify.erase(toModify.begin(), toModify.begin() + (int)leftPrefix.size());
		return ITrackedDevice::HAND_LEFT;
	}

	if (toModify.starts_with(rightPrefix)) {
		toModify.erase(toModify.begin(), toModify.begin() + (int)rightPrefix.size());
		return ITrackedDevice::HAND_RIGHT;
	}

	return ITrackedDevice::HAND_NONE;
}
