#include "IndexControllerInteractionProfile.h"
#include "logging.h"

#include "IndexHandPoses.h"
#include "Reimpl/BaseInput.h"

IndexControllerInteractionProfile::IndexControllerInteractionProfile()
{
	// Figured these out from OpenXR spec section 6.4
	const char* paths[] = {
		// Runtimes are not required to support the system button paths, and no OpenVR game can use them anyway.
		//"/input/system/click",
		//"/input/system/touch",
		"/input/a/click",
		"/input/a/touch",
		"/input/b/click",
		"/input/b/touch",
		"/input/squeeze/value",
		"/input/squeeze/force",
		"/input/trigger/click",
		"/input/trigger/value",
		"/input/trigger/touch",
		"/input/thumbstick/x",
		"/input/thumbstick/y",
		"/input/thumbstick/click",
		"/input/thumbstick/touch",
		"/input/thumbstick", // Allows directly binding 2 axis inputs like OpenVR does - see section 11.4 of the OpenXR spec
		"/input/trackpad/x",
		"/input/trackpad/y",
		"/input/trackpad/force",
		"/input/trackpad/touch",
		"/input/trackpad",
		"/input/grip/pose",
		"/input/aim/pose",
		"/output/haptic"
	};

	for (const auto& path : paths) {
		this->validInputPaths.insert("/user/hand/left" + std::string(path));
		this->validInputPaths.insert("/user/hand/right" + std::string(path));
	}

	// These are substring replacements which translate from OpenVR paths to OpenXR paths.
	// I found the necessary examples of OpenVR paths in ./steamapps/common/NeosVR/Neos_Data/StreamingAssets/SteamVR/bindings_knuckles.json
	this->pathTranslationMap = {
		{ "thumbstick/position", "thumbstick" },
		{ "trackpad/position", "trackpad" },
		{ "trackpad/click", "trackpad/touch" },

		{ "pull", "value" },
		{ "application_menu", "system" },

		// All of these are "value" and not "force", because to activate "force" all the way
		// you have to pull really hard which is nerve racking
		// If you look at the bindings_knuckles.json you can see some more compliated thresholds, which I believe OC ignores.
		// If we add support for respecting those, change these to more reasonable replacements.
		{ "grip/force", "squeeze/value" },
		{ "grip/click", "squeeze/value" },
		{ "grip/touch", "squeeze/value" },
		{ "grip/value", "squeeze/value" },
		{ "grip/grab", "squeeze/value" },
		{ "grip/pull", "squeeze/value" },
	};

	this->hmdPropertiesMap = {
		// { vr::Prop_TrackingSystemName_String, "lighthouse" },
		{ vr::Prop_ManufacturerName_String, "Valve" },
	};

	this->propertiesMap = {
		// { vr::Prop_TrackingSystemName_String, { "lighthouse" } },
		{ vr::Prop_ModelNumber_String, { "Knuckles Left", "Knuckles Right" } },
		{ vr::Prop_ControllerType_String, { GetOpenVRName().value() } }
	};

	/*
	 * Values used to create this found here:
	 * SteamVR\drivers\indexcontroller\resources\rendermodels\valve_controller_knu_1_0_left\valve_controller_knu_1_0_left.json
	 */
	glm::mat4 inverseGripTransformLeft = GetMat4x4FromOriginAndEulerRotations(
	    { 0.0, -0.015, 0.13 },
	    { 15.392, -2.071, 0.303 });

	/*
	 * Values used to create this found here:
	 * SteamVR\drivers\indexcontroller\resources\rendermodels\valve_controller_knu_1_0_right\valve_controller_knu_1_0_right.json
	 */
	glm::mat4 inverseGripTransformRight = GetMat4x4FromOriginAndEulerRotations(
	    { 0.0, -0.015, 0.13 },
	    { 15.392, 2.071, -0.303 });

	this->leftHandGripTransform = glm::affineInverse(inverseGripTransformLeft);
	this->rightHandGripTransform = glm::affineInverse(inverseGripTransformRight);

	// Set up the component transforms

	glm::mat4 bodyLeft = GetMat4x4FromOriginAndEulerRotations(
	    { -0.005154, 0.013042, 0.107171 },
	    { 93.782, 0.0, 0.0 });
	glm::mat4 bodyRight = GetMat4x4FromOriginAndEulerRotations(
	    { 0.005154, 0.013042, 0.107171 },
	    { 93.782, 0.0, 0.0 });
	glm::mat4 tipLeft = GetMat4x4FromOriginAndEulerRotations(
	    { 0.006, -0.015, 0.02 },
	    { -40.0, -5.0, 0.0 });
	glm::mat4 tipRight = GetMat4x4FromOriginAndEulerRotations(
	    { -0.006, -0.015, 0.02 },
	    { -40.0, 5.0, 0.0 });
	glm::mat4 baseLeft = GetMat4x4FromOriginAndEulerRotations(
	    { 0.004758, -0.037977, 0.200466 },
	    { -155.4, -0.427, 7.081 });
	glm::mat4 baseRight = GetMat4x4FromOriginAndEulerRotations(
	    { -0.004758, -0.037977, 0.200466 },
	    { -155.4, -0.427, -7.081 });
	glm::mat4 gdc = GetMat4x4FromOriginAndEulerRotations(
	    { 0.0, 0.0, 0.0 },
	    { 0.0, 0.0, 0.0 });

	this->leftComponentTransforms["body"] = glm::affineInverse(bodyLeft);
	this->rightComponentTransforms["body"] = glm::affineInverse(bodyRight);
	this->leftComponentTransforms["tip"] = glm::affineInverse(tipLeft);
	this->rightComponentTransforms["tip"] = glm::affineInverse(tipRight);
	this->leftComponentTransforms["base"] = glm::affineInverse(baseLeft);
	this->rightComponentTransforms["base"] = glm::affineInverse(baseRight);
	this->leftComponentTransforms["gdc2015"] = glm::affineInverse(gdc);
	this->rightComponentTransforms["gdc2015"] = glm::affineInverse(gdc);

	// Define reference poses
	leftHandPoses[VRSkeletalReferencePose_BindPose] = knuckles::leftBindPose;
	leftHandPoses[VRSkeletalReferencePose_OpenHand] = knuckles::leftOpenHandPose;
	leftHandPoses[VRSkeletalReferencePose_Fist] = knuckles::leftFistPose;
	leftHandPoses[VRSkeletalReferencePose_GripLimit] = knuckles::leftGripLimitPose;

	rightHandPoses[VRSkeletalReferencePose_BindPose] = knuckles::rightBindPose;
	rightHandPoses[VRSkeletalReferencePose_OpenHand] = knuckles::rightOpenHandPose;
	rightHandPoses[VRSkeletalReferencePose_Fist] = knuckles::rightFistPose;
	rightHandPoses[VRSkeletalReferencePose_GripLimit] = knuckles::rightGripLimitPose;

}

const std::string& IndexControllerInteractionProfile::GetPath() const
{
	static std::string path = "/interaction_profiles/valve/index_controller";
	return path;
}

std::optional<const char*> IndexControllerInteractionProfile::GetLeftHandRenderModelName() const
{
	return "{indexcontroller}valve_controller_knu_1_0_left";
}

std::optional<const char*> IndexControllerInteractionProfile::GetRightHandRenderModelName() const
{
	return "{indexcontroller}valve_controller_knu_1_0_right";
}

std::optional<const char*> IndexControllerInteractionProfile::GetOpenVRName() const
{
	return "knuckles";
}

std::optional<vr::EVRSkeletalTrackingLevel> IndexControllerInteractionProfile::GetOpenVRTrackinglevel() const
{
	return vr::VRSkeletalTracking_Partial;
}

const InteractionProfile::LegacyBindings* IndexControllerInteractionProfile::GetLegacyBindings(const std::string& handPath) const
{
	// Index controllers are exactly symmetrical, so we can just drop handPath on the floor.
	static LegacyBindings bindings = {};
	//	this->bindingsLegacy.system = "input/system/click"; - causes issues on Oculus runtime
	bindings.menu = "input/b/click";
	bindings.menuTouch = "input/b/touch";
	bindings.btnA = "input/a/click";
	bindings.btnATouch = "input/a/touch";

	bindings.trackpadX = "input/trackpad/x";
	bindings.trackpadY = "input/trackpad/y";
	bindings.trackpadTouch = "input/trackpad/touch";
	bindings.trackpadForce = "input/trackpad/force";
	bindings.stickX = "input/thumbstick/x";
	bindings.stickY = "input/thumbstick/y";
	bindings.stickBtn = "input/thumbstick/click";
	bindings.stickBtnTouch = "input/thumbstick/touch";
	bindings.trigger = "input/trigger/value";
	bindings.triggerClick = "input/trigger/click";
	bindings.triggerTouch = "input/trigger/touch";

	bindings.grip = "input/squeeze/value";
	bindings.haptic = "output/haptic";

	bindings.gripPoseAction = "input/grip/pose";
	bindings.aimPoseAction = "input/aim/pose";
	return &bindings;
}
