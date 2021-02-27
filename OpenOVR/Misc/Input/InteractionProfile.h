//
// Created by ZNix on 27/02/2021.
//

#pragma once

#include <set>
#include <string>
#include <vector>

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
	 */
	virtual const std::vector<std::string>& GetValidInputPaths() const = 0;

	virtual bool IsInputPathValid(const std::string& inputPath) const = 0;
};

class OculusTouchInteractionProfile : public InteractionProfile {
public:
	OculusTouchInteractionProfile();

	const std::string& GetPath() const override { return path; }
	const std::vector<std::string>& GetValidInputPaths() const override { return validInputPaths; }

	bool IsInputPathValid(const std::string& inputPath) const override { return validInputPathsSet.count(inputPath) > 0; }

private:
	std::string path;
	std::vector<std::string> validInputPaths;
	std::set<std::string> validInputPathsSet;
};
