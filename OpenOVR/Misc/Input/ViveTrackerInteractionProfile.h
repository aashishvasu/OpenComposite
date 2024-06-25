#pragma once

#include "InteractionProfile.h"

class ViveTrackerInteractionProfile : public InteractionProfile {
public:
	ViveTrackerInteractionProfile();

	const std::string& GetPath() const override;
	std::optional<const char*> GetLeftHandRenderModelName() const override;
	std::optional<const char*> GetRightHandRenderModelName() const override;
	std::optional<const char*> GetOpenVRName() const override;
	bool CanHaveBindings() const override;

protected:
	LegacyBindings bindingsLegacy = {};
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;
	std::string role;
};
