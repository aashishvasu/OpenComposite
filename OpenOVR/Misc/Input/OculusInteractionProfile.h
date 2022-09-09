//
// Created by ZNix on 24/03/2021.
//

#pragma once

#include "InteractionProfile.h"

class OculusTouchInteractionProfile : public InteractionProfile {
public:
	OculusTouchInteractionProfile();

	const std::string& GetPath() const override;
	const char* GetOpenVRName() const override;

protected:
	const LegacyBindings* GetLegacyBindings(const std::string& handPath) const override;
};
