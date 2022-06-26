//
// Created by ZNix on 27/02/2021.
//

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "InputData.h"
#include "Reimpl/BaseInput.h"

/**
 * Represents an input that doesn't exist in the OpenXR runtime but does in SteamVR. These
 * read an OpenXR input then convert it.
 *
 * An example is the 'click' action for analogue inputs like the trigger or grip. They don't
 * exist in Oculus's bindings but do in SteamVR, and this provides them.
 */
class VirtualInput {
public:
	/**
	 * Describes this binding and sets how it should be used to build OpenXR actions.
	 */
	struct BindInfo {
		XrActionSet actionSet;
		std::string actionSetName;
		std::string openvrActionName; // The name of the openvr action which is bound to this input
		std::string localisedName;
	};

	explicit VirtualInput(BindInfo info);

	virtual vr::EVRInputError GetDigitalActionData(OOVR_InputDigitalActionData_t* pActionData);

	void AddSuggestedBindings(std::vector<XrActionSuggestedBinding>& bindings);

	/**
	 * Call when BaseInput is done setting everything else up, and we can do our last setup steps.
	 */
	void PostInit();

	/**
	 * Should be called directly before xrSyncActions. Updates the data needed to write bChanged (or equivalents for
	 * non-digital actions).
	 */
	void OnPreFrame();

	/**
	 * Get a list of physical actions this virtual action contains, to be used in GetActionOrigins.
	 */
	const std::vector<XrAction>& GetActionsForOriginLookup() const;

protected:
	virtual void Update() = 0;

	XrAction CreateAction(const std::string& pathSuffix, XrActionType type, const std::string& localisedNameSuffix);

	std::vector<XrActionSuggestedBinding> suggestedBindings;

protected:
	BindInfo bindInfo;

	// The SyncSerial from BaseInput that these are valid for
	uint64_t digitalSerial = 0;

	OOVR_InputDigitalActionData_t digital = {};

	std::vector<XrAction> actions;

	/**
	 * The good activeOrigin value for all our produced outputs. This will be applied by InteractionProfile and
	 * subclasses don't need to set it.
	 */
	vr::VRInputValueHandle_t activeOrigin = vr::k_ulInvalidInputValueHandle;

private:
	/**
	 * The counter for a unique number to be used in each action name to avoid collisions.
	 */
	static int actionSerial;
};

/**
 * Defines a factory that can construct a VirtualInput for a given action set.
 *
 * This is required because while in InteractionProfile defines what virtual
 * inputs exist and the actions they map to, it cannot define which ActionSets
 * they're used on since that depends on the game.
 */
class VirtualInputFactory {
public:
	typedef std::function<std::unique_ptr<VirtualInput>(const VirtualInput::BindInfo&)> builder_t;

	VirtualInputFactory(std::string name, builder_t builder);

	inline std::unique_ptr<VirtualInput> BuildFor(const VirtualInput::BindInfo& info) const { return builder(info); }

	inline const std::string& GetName() const { return name; }

private:
	std::string name;
	builder_t builder;
};

/**
 * Defines an interaction profile, as specified by 6.4 in the OpenXR spec.
 */
class InteractionProfile {
public:
	virtual ~InteractionProfile() = default;

	/**
	 * Get the path of the profile as used by xrSuggestInteractionProfileBindings, for
	 * example /interaction_profiles/khr/simple_controller.
	 */
	virtual const std::string& GetPath() const = 0;

	/**
	 * Gets a list of valid input paths for this profile. For example, on the simple controller:
	 *
	 * /user/hand/left/input/select/click
	 * /user/hand/left/input/menu/click
	 * /user/hand/left/input/grip/pose
	 * /user/hand/left/input/aim/pose
	 * /user/hand/left/output/haptic
	 * /user/hand/right/input/select/click
	 * /user/hand/right/input/menu/click
	 * /user/hand/right/input/grip/pose
	 * /user/hand/right/input/aim/pose
	 * /user/hand/right/output/haptic
	 *
	 * Note this does not include virtual inputs.
	 */
	virtual const std::unordered_set<std::string>& GetValidInputPaths() const = 0;

	/**
	 * Returns true if the given path is present in GetValidInputsPaths.
	 *
	 * This does not include virtual inputs.
	 */
	virtual bool IsInputPathValid(const std::string& inputPath) const = 0;

	/**
	 * Get the list of VirtualInputFactories representing all the virtual inputs supported
	 * by this profile.
	 *
	 * NOTE: The storage for these items must not move, as references will be made to them!
	 */
	virtual const std::vector<VirtualInputFactory>& GetVirtualInputs() const = 0;

	virtual const VirtualInputFactory* GetVirtualInput(const std::string& inputPath) const;

	/**
	 * Translate an unsupported path to a supported one.
	 * For example, for the simple controller, this will translate any trigger paths to select paths.
	 */
	virtual std::string TranslateAction(const std::string& inputPath) const;

	/**
	 * Build a list of suggested bindings for attaching the legacy actions to this profile.
	 */
	void AddLegacyBindings(const BaseInput::LegacyControllerActions& actions, std::vector<XrActionSuggestedBinding>& bindings) const;

protected:
	struct LegacyBindings {
		// Matches up with BaseInput::LegacyControllerActions - see it for comments
		// Specifies the path for each action
		// These paths are relative to the hand - eg, use input/trigger/value not /user/hand/left/input/trigger/value
		const char* system = nullptr;
		const char *menu = nullptr, *menuTouch = nullptr;
		const char *btnA = nullptr, *btnATouch = nullptr;

		const char *stickX = nullptr, *stickY = nullptr, *stickBtn = nullptr, *stickBtnTouch = nullptr;
		const char *trigger = nullptr, *triggerTouch = nullptr;
		const char* grip = nullptr;

		const char* haptic = nullptr;

		const char *gripPoseAction = nullptr, *aimPoseAction = nullptr;
	};

	/**
	 * Finish setting up this instance.
	 *
	 * PostSetup MUST be by the superclass once it's virtual methods will return their final values.
	 */
	void PostSetup();

	virtual const LegacyBindings* GetLegacyBindings(const std::string& handPath) const = 0;

private:
	std::map<std::string, const VirtualInputFactory*> virtualInputNames;

	bool donePostSetup = false;
};
