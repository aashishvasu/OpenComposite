//
// Created by ZNix on 24/03/2021.
//

#pragma once

#include "InteractionProfile.h"

class OculusTouchInteractionProfile : public InteractionProfile {
public:
	OculusTouchInteractionProfile();

	const std::string& GetPath() const override;
	const std::unordered_set<std::string>& GetValidInputPaths() const override;
	bool IsInputPathValid(const std::string& inputPath) const override;
	const std::vector<VirtualInputFactory>& GetVirtualInputs() const override;
	const char* GetOVRName() const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;

private:
	std::string path;
	std::unordered_set<std::string> validInputPaths;
	std::vector<VirtualInputFactory> virtualInputs;
};
