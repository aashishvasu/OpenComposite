//
// Created by ZNix on 27/02/2021.
//

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "Drivers/Backend.h"
#include "InputData.h"
#include "LegacyControllerActions.h"
/**
 * Defines an interaction profile, as specified by 6.4 in the OpenXR spec.
 * Implementing an interaction profile is fairly straightforward, view the other interaction profiles for examples.
 * Be sure to add any implemented profiles to the interactionProfiles vector in BaseInput.cpp. (This is done in the BaseInput constructor.)
 */
class InteractionProfile {
private:
	// types of properties that can be retrieved
	using property_types = std::variant<
	    bool, float, int32_t, uint64_t, vr::HmdMatrix34_t,
	    const std::vector<uint32_t>, // array
	    std::string // string
	    >;

	// helper struct for determining if a type is in our list of allowed types (property_types)
	template <typename T, typename U>
	struct in_variant : std::false_type {
	};

	template <typename T, typename... Ts>
	struct in_variant<T, std::variant<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {
	};

	// helper struct for ordering translationMap
	struct translation_compare {
		bool operator()(const std::string& lhs, const std::string& rhs) const
		{
			if (lhs.size() != rhs.size())
				return lhs.size() > rhs.size();
			else
				// without this, strings that are the same length will be considered the same by the map
				return lhs > rhs;
		}
	};

public:
	virtual ~InteractionProfile() = default;

	using ProfileList = std::vector<std::unique_ptr<InteractionProfile>>;
	/**
	 * Gets a list of all of the possible interaction profiles.
	 * If implementing a new profile, be sure to add it here!
	 */
	static const ProfileList& GetProfileList();

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
	 */
	const std::unordered_set<std::string>& GetValidInputPaths() const;

	/**
	 * Returns true if the given path is present in GetValidInputsPaths.
	 */
	bool IsInputPathValid(const std::string& inputPath) const;

	/**
	 * Translate an unsupported path to a supported one using the pathTranslationMap.
	 * For example, for the simple controller, this will translate any trigger paths to select paths.
	 */
	std::string TranslateAction(const std::string& inputPath) const;

	/**
	 * Returns the name for the profile as recognized by OpenVR, if it is recognized by it.
	 */
	virtual std::optional<const char*> GetOpenVRName() const = 0;

	/**
	 * Returns the transform matrix from grip space to the SteamVR controller pose.
	 *
	 * This only affects games which use the controller's pose directly, without using
	 * BaseRenderModels::GetComponentState to find the offset of a component they care
	 * about, such as the aim or grip component.
	 *
	 * This should solve a string fairly longstanding issue, most infamously with Boneworks:
	 * https://gitlab.com/znixian/OpenOVR/-/issues/152
	 *
	 * This is quite ugly since it's inherently controller-specific.
	 *
	 * By default, this returns the identity matrix.
	 */
	virtual glm::mat4 GetGripToSteamVRTransform(ITrackedDevice::HandType hand) const;

	/**
	 * Build a list of suggested bindings for attaching the legacy actions to this profile.
	 */
	void AddLegacyBindings(const LegacyControllerActions& actions, std::vector<XrActionSuggestedBinding>& bindings) const;

	/**
	 * Get the requested property from the profile, if it exists.
	 * If hand == HAND_NONE, this will retrieve the HMD version of the property.
	 */
	template <typename T>
	requires(in_variant<T, property_types>::value)
	    std::optional<T> GetProperty(vr::ETrackedDeviceProperty property, ITrackedDevice::HandType hand)
	const
	{
		using enum ITrackedDevice::HandType;
		if (hand != HAND_NONE && propertiesMap.contains(property)) {
			hand_values_type ret = propertiesMap.at(property);
			return std::get<T>((hand == HAND_RIGHT && ret.right.has_value()) ? ret.right.value() : ret.left);
		} else if (hmdPropertiesMap.contains(property)) {
			return std::get<T>(hmdPropertiesMap.at(property));
		}
		return std::nullopt;
	}

protected:
	struct LegacyBindings {
		// Matches up with LegacyControllerActions - see it for comments
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

	/*
	 * Returns a legacy bindings struct for the given interaction profile.
	 */
	virtual const LegacyBindings* GetLegacyBindings(const std::string& handPath) const = 0;

	// The set of valid input paths for an interaction profile. An interaction profile should fill this in its constructor.
	std::unordered_set<std::string> validInputPaths;

	// A map with OpenVR action name parts as keys and OpenXR equivalents as values.
	// For example, one common key, value pair might be "application_menu", "menu"
	std::map<std::string, std::string, translation_compare> pathTranslationMap;

	// A map for HMD properties.
	// Note that for a SteamVR supported device can be extracted from the SteamVR System Report,
	// in the properties.json section
	// HMD Properties an interaction profile should implement:
	// - Prop_ManufacturerName_String
	std::unordered_map<vr::ETrackedDeviceProperty, property_types> hmdPropertiesMap;

	template <typename T>
	struct hand_values_type {
		T left;
		std::optional<T> right;
	};

	// A map for controller properties
	// If a value for the right hand isn't provided, the one for the left hand will be used.
	// Controller Properties an interaction profile should implement:
	// - Prop_ModelNumber_String
	// - Prop_ControllerType_String
	std::unordered_map<vr::ETrackedDeviceProperty, hand_values_type<property_types>> propertiesMap;
};
