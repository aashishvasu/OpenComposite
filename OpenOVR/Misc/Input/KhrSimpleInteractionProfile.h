//
// Created by znix on 17/04/2022.
//

#pragma once

#include "InteractionProfile.h"

class KhrSimpleInteractionProfile : public InteractionProfile {
public:
	KhrSimpleInteractionProfile();

	const std::string& GetPath() const override;
	std::optional<const char*> GetLeftHandRenderModelName() const override;
	std::optional<const char*> GetRightHandRenderModelName() const override;
	std::optional<const char*> GetOpenVRName() const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;
};
