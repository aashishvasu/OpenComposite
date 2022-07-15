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
	const std::unordered_set<std::string>& GetValidInputPaths() const;

	/**
	 * Returns true if the given path is present in GetValidInputsPaths.
	 *
	 * This does not include virtual inputs.
	 */
	bool IsInputPathValid(const std::string& inputPath) const;

	/**
	 * Translate an unsupported path to a supported one using the pathTranslationMap.
	 * For example, for the simple controller, this will translate any trigger paths to select paths.
	 */
	std::string TranslateAction(const std::string& inputPath) const;

	/**
	 * Returns the name for the profile as recognized by OpenVR.
	 * If null, this device must not be recognized by OpenVR.
	 */
	virtual const char* GetOpenVRName() const = 0;

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

	virtual const LegacyBindings* GetLegacyBindings(const std::string& handPath) const = 0;

	// The set of valid input paths for an interaction profile. An interaction profile should fill this in its constructor.
	std::unordered_set<std::string> validInputPaths;

	// A map with OpenVR action name parts as keys and OpenXR equivalents as values.
	// For example, one common key, value pair might be "application_menu", "menu"
	std::unordered_map<std::string, std::string> pathTranslationMap;

private:
	bool donePostSetup = false;
};
