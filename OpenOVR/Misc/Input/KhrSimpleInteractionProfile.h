//
// Created by znix on 17/04/2022.
//

#pragma once

#include "InteractionProfile.h"

class KhrSimpleInteractionProfile : public InteractionProfile {
public:
	KhrSimpleInteractionProfile();

	const std::string& GetPath() const override;
	const std::unordered_set<std::string>& GetValidInputPaths() const override;
	bool IsInputPathValid(const std::string& inputPath) const override;
	const std::vector<VirtualInputFactory>& GetVirtualInputs() const override;
	std::string TranslateAction(const std::string& inputPath) const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;

private:
	std::unordered_set<std::string> validPaths;
	std::vector<VirtualInputFactory> virtualInputs;
};
