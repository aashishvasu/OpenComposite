#include "ViveTrackerInteractionProfile.h"
#include "logging.h"

ViveTrackerInteractionProfile::ViveTrackerInteractionProfile()
{
	propertiesMap = {
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } }
	};
}

const std::string& ViveTrackerInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/htc/vive_tracker_htcx";
	return path;
}

std::optional<const char*> ViveTrackerInteractionProfile::GetLeftHandRenderModelName() const
{
	return GetOpenVRName().value();
}

std::optional<const char*> ViveTrackerInteractionProfile::GetRightHandRenderModelName() const
{
	return GetOpenVRName().value();
}

const InteractionProfile::LegacyBindings* ViveTrackerInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	static LegacyBindings bindings = {};

	return &bindings;
}

std::optional<const char*> ViveTrackerInteractionProfile::GetOpenVRName() const
{
	return "vive_tracker";
}

bool ViveTrackerInteractionProfile::CanHaveBindings() const
{
	// generic trackers do not support any actions, therefore no bindings.
	return false;
}
