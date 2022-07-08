#pragma once

#include "InteractionProfile.h"

class ViveWandInteractionProfile : public InteractionProfile {
public:
	ViveWandInteractionProfile();

	const std::string& GetPath() const override;
	const char* GetOpenVRName() const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;
};
