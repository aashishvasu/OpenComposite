#include "Misc/xrmoreutils.h"
#include "stdafx.h"
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#define BASE_IMPL
#include "BaseInput.h"
#include "BaseInput_HandPoses.hpp"

#include <convert.h>

#include <glm/ext.hpp>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>

#include <optional>
#include <ranges>

// Think of this file as just a part of BaseInput.cpp.
// It's split out in pursuit of lower compile times.

using namespace vr;

// MODEL POSE STUFF
template <class T, class U>
static void quaternionCopy(const T& src, U& dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
	dst.w = src.w;
}

// UTILITY FUNCTIONS
static glm::quat ApplyBoneHandTransform(glm::quat quat, bool isRight)
{
	std::swap(quat.x, quat.z);
	quat.z *= -1.f;
	if (isRight) {
		return quat;
	}

	quat.x *= -1.f;
	quat.y *= -1.f;
	return quat;
}

static bool isBoneMetacarpal(XrHandJointEXT handJoint)
{
	return handJoint == XR_HAND_JOINT_THUMB_METACARPAL_EXT || handJoint == XR_HAND_JOINT_INDEX_METACARPAL_EXT || handJoint == XR_HAND_JOINT_MIDDLE_METACARPAL_EXT || handJoint == XR_HAND_JOINT_RING_METACARPAL_EXT || handJoint == XR_HAND_JOINT_LITTLE_METACARPAL_EXT;
}

static std::map<HandSkeletonBone, XrHandJointEXT> auxBoneMap = {
	{ eBone_Aux_Thumb, XR_HAND_JOINT_THUMB_DISTAL_EXT },
	{ eBone_Aux_IndexFinger, XR_HAND_JOINT_INDEX_DISTAL_EXT },
	{ eBone_Aux_MiddleFinger, XR_HAND_JOINT_MIDDLE_DISTAL_EXT },
	{ eBone_Aux_RingFinger, XR_HAND_JOINT_RING_DISTAL_EXT },
	{ eBone_Aux_PinkyFinger, XR_HAND_JOINT_LITTLE_DISTAL_EXT },
};

constexpr vr::VRBoneTransform_t leftOpenPose[2] = {
	{ { 0.000000f, 0.000000f, 0.000000f, 1.000000f }, { 1.000000f, 0.000000f, 0.000000f, 0.000000f } },
	{ { -0.034038f, 0.036503f, 0.164722f, 1.000000f }, { -0.055147f, -0.078608f, -0.920279f, 0.379296f } },
};

constexpr vr::VRBoneTransform_t rightOpenPose[2] = {
	{ { 0.000000f, 0.000000f, 0.000000f, 1.000000f }, { 1.000000f, -0.000000f, -0.000000f, 0.000000f } },
	{ { 0.034038f, 0.036503f, 0.164722f, 1.000000f }, { -0.055147f, -0.078608f, 0.920279f, -0.379296f } },
};

constexpr XrHandJointEXT metacarpalJoints[5] = {
	XR_HAND_JOINT_THUMB_METACARPAL_EXT,
	XR_HAND_JOINT_INDEX_METACARPAL_EXT,
	XR_HAND_JOINT_MIDDLE_METACARPAL_EXT,
	XR_HAND_JOINT_RING_METACARPAL_EXT,
	XR_HAND_JOINT_LITTLE_METACARPAL_EXT
};

// Transform bones to be relative to their parent
static bool GetJointRelative(const XrHandJointLocationEXT& currentJoint, const XrHandJointLocationEXT& parentJoint, bool isRight, vr::VRBoneTransform_t& outBoneTransform)
{
	glm::quat parentOrientationInverse = glm::conjugate(X2G_quat(parentJoint.pose.orientation));
	glm::quat orientation = parentOrientationInverse * X2G_quat(currentJoint.pose.orientation);

	// get us in the OpenVR coordinate space
	orientation = ApplyBoneHandTransform(orientation, isRight);
	quaternionCopy(orientation, outBoneTransform.orientation);

	glm::vec3 position = X2G_v3f(currentJoint.pose.position) - X2G_v3f(parentJoint.pose.position);

	outBoneTransform.position = { glm::length(position), 0.f, 0.f, 1 };

	if (isRight) {
		outBoneTransform.position.v[0] *= -1;
	}

	return true;
}

static bool MetacarpalJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output)
{
	for (int joint : metacarpalJoints) {

		const XrHandJointLocationEXT& currentJoint = joints[joint];
		const XrHandJointLocationEXT& parentJoint = joints[XR_HAND_JOINT_WRIST_EXT];

		if (currentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0) || parentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0))
			return false;

		// Everything has to be done relative to its parent.
		// It was previously believed this matters if the application doesn't request this, but it doesn't.
		// The only Meaningful change is that the hand follows the grip pose of the controller if the `EVRSkeletalTransformSpace` is `VRSkeletalTransformSpace_Model`.
		glm::quat parentOrientationInverse = glm::inverse(X2G_quat(parentJoint.pose.orientation));
		glm::quat orientationXr = parentOrientationInverse * X2G_quat(currentJoint.pose.orientation);

		// get us in the OpenVR coordinate space
		glm::quat orientationVr = ApplyBoneHandTransform(orientationXr, isRight);

		// something to do with Maya apparently, unsure how this works. Something about render models?
		glm::quat magicRotation = glm::quat(0.5f, isRight ? -0.5f : 0.5f, isRight ? 0.5f : -0.5f, 0.5f);
		orientationVr = magicRotation * orientationVr;

		quaternionCopy(orientationVr, output[joint].orientation);

		glm::vec3 position = X2G_v3f(currentJoint.pose.position) - X2G_v3f(parentJoint.pose.position);
		position = glm::rotate(parentOrientationInverse, position);

		output[joint].position = { position.y, position.x, -position.z, 1.f }; // What's the fourth value for?

		if (isRight) {
			output[joint].position.v[0] *= -1;
			output[joint].position.v[1] *= -1;
		}
	}

	return true;
}

static bool FlexionJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output)
{
	int parentId = -1;

	for (int joint = XR_HAND_JOINT_THUMB_METACARPAL_EXT; joint < XR_HAND_JOINT_COUNT_EXT; joint++) {
		if (isBoneMetacarpal((XrHandJointEXT)joint)) {
			parentId = joint;
			continue;
		}

		const XrHandJointLocationEXT& currentJoint = joints[joint];
		const XrHandJointLocationEXT& parentJoint = joints[parentId];

		if (currentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0) || parentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0))
			return false;

		if (!GetJointRelative(currentJoint, parentJoint, isRight, output[joint]))
			return false;

		parentId = joint;
	}

	return true;
}

static bool AuxJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output)
{
	XrHandJointLocationEXT currentJoint;
	for (int i = eBone_Aux_Thumb; i <= eBone_Aux_PinkyFinger; i++) {
		currentJoint = joints[auxBoneMap[(HandSkeletonBone)i]];

		if (currentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0) || joints[XR_HAND_JOINT_WRIST_EXT].locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0))
			return false;

		if (!GetJointRelative(currentJoint, joints[XR_HAND_JOINT_WRIST_EXT], isRight, output[i]))
			return false;
	}

	return true;
}

static void InterpolateBone(VRBoneTransform_t& bone, const VRBoneTransform_t& targetBone, float pct, bool slerp = false) {
	glm::vec3 startPos(bone.position.v[0], bone.position.v[1], bone.position.v[2]);
	glm::quat startRot(bone.orientation.w, bone.orientation.x, bone.orientation.y, bone.orientation.z);

	glm::vec3 targetPos(targetBone.position.v[0], targetBone.position.v[1], targetBone.position.v[2]);
	glm::quat targetRot(targetBone.orientation.w, targetBone.orientation.x, targetBone.orientation.y, targetBone.orientation.z);

	glm::vec3 resPos = glm::mix(startPos, targetPos, pct);
	glm::quat resRot;
	if (slerp)
		resRot = glm::slerp(startRot, targetRot, pct);
	else
		resRot = glm::mix(startRot, targetRot, pct);

	bone.position = { resPos.x, resPos.y, resPos.z, 1.f };
	bone.orientation = { resRot.w, resRot.x, resRot.y, resRot.z };
}

// OpenXR Hand Joints to OpenVR Hand Skeleton logic generously donated by danwillm from valve.
bool BaseInput::XrHandJointsToSkeleton(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output)
{
	for (int i : { XR_HAND_JOINT_PALM_EXT, XR_HAND_JOINT_WRIST_EXT }) {
		output[i] = isRight ? rightOpenPose[i] : leftOpenPose[i];
	}

	if (!MetacarpalJointPass(joints, isRight, output))
		return false;

	if (!FlexionJointPass(joints, isRight, output))
		return false;

	if (!AuxJointPass(joints, isRight, output))
		return false;

	return true;
}

namespace {
// The estimated poses are given in terms of root being located at (0, 0, 0), i.e, the wrist will
// start where the grip pose starts. However, the OpenXR spec specifies that the grip pose is
// supposed to roughly line up with the palm centroid, so this (arbitrary) constant will
// slide the bone poses back to meet this expectation.
constexpr float handZDisplacement = 0.2;
// How much trigger/grip needs to be pressed before simulating fingers curling
constexpr float simulateCurlThreshold = 0.08;

} // namespace

EVRInputError BaseInput::getEstimatedBoneData(
    ITrackedDevice::TrackedDeviceType hand,
    EVRSkeletalTransformSpace transformSpace,
    std::span<VRBoneTransform_t, eBone_Count> boneData)
{
	auto& controller = legacyControllers[hand];

		XrActionStateGetInfo info{
		.type = XR_TYPE_ACTION_STATE_GET_INFO,
		.action = controller.trigger,
		.subactionPath = XR_NULL_PATH,
	};
	XrActionStateFloat state{ XR_TYPE_ACTION_STATE_FLOAT };
	XrActionStateBoolean stateBool{ XR_TYPE_ACTION_STATE_BOOLEAN };

	XrResult actionRes = xrGetActionStateFloat(xr_session.get(), &info, &state);
	float triggerPct = 0.0f;
	if (XR_SUCCEEDED(actionRes) && state.currentState >= simulateCurlThreshold) {
		triggerPct = state.currentState;
	} else if (XR_FAILED(actionRes)) {
		OOVR_LOGF("WARNING: couldn't get trigger percentage (%d)", actionRes);
	}

	info.action = controller.grip;
	actionRes = xrGetActionStateFloat(xr_session.get(), &info, &state);
	float gripPct = 0.0f;
	if (XR_SUCCEEDED(actionRes) && state.currentState >= simulateCurlThreshold) {
		gripPct = state.currentState;
	} else if (XR_FAILED(actionRes)) {
		OOVR_LOGF("WARNING: couldn't get grip percentage (%d)", actionRes);
	}

	info.action = controller.triggerTouch;
	actionRes = xrGetActionStateBoolean(xr_session.get(), &info, &stateBool);
	bool triggerTouch = false;
	if (XR_SUCCEEDED(actionRes)) {
		triggerTouch = stateBool.currentState;
	}

	bool hasThumbInput = false;
	bool thumbTouch = false;

	// Put thumb down on any relevant touch inputs
	const XrAction thumbActions[] = {controller.menuTouch, controller.btnATouch, controller.trackpadTouch, controller.stickBtnTouch};

	for (const auto& action : thumbActions) {
		info.action = action;
		actionRes = xrGetActionStateBoolean(xr_session.get(), &info, &stateBool);
		if (XR_SUCCEEDED(actionRes)) {
			if (stateBool.currentState) {
				thumbTouch = true;
			}
			hasThumbInput = true;
		}
	}
	
	// Allow for straightening the thumb on controllers with no touch inputs
	if (!hasThumbInput)
		thumbTouch = gripPct != 1.0f || triggerPct == 1.0f;

	// Calculate deltaTime for binary input interpolation (per hand)
	static std::array<std::chrono::high_resolution_clock::time_point, 2> lastTime{
		std::chrono::high_resolution_clock::now(), 
		std::chrono::high_resolution_clock::now()
	};

	auto currentTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<float> deltaTimeDuration = currentTime - lastTime[hand];
	float deltaTime = deltaTimeDuration.count();

	lastTime[hand] = currentTime;

	// Store interpolated floats for thumb and trigger
	static std::array<float, 2> thumbTouchPcts = {0.0f, 0.0f}; 
	static std::array<float, 2> triggerTouchPcts = {0.0f, 0.0f}; 
	
	const float touchTransitionSpeed = 8.0f;
			
	// Update floats using deltaTime
	thumbTouchPcts[hand] = glm::clamp(thumbTouchPcts[hand] + (thumbTouch ? deltaTime * touchTransitionSpeed : -deltaTime * touchTransitionSpeed), 0.0f, 1.0f);
	triggerTouchPcts[hand] = glm::clamp(triggerTouchPcts[hand] + (triggerTouch ? deltaTime * touchTransitionSpeed : -deltaTime * touchTransitionSpeed), 0.0f, 1.0f);

	float thumbTouchPct = thumbTouchPcts[hand];
	float triggerTouchPct = triggerTouchPcts[hand];

	auto boneDataGen = [triggerPct, gripPct, triggerTouchPct, thumbTouchPct](const BoneArray& bindPose, const BoneArray& squeezePose, const BoneArray& openHandPose) {
		return std::ranges::iota_view(0, static_cast<int>(eBone_Count)) | std::views::transform([=](int bone_index) {
			auto bone = openHandPose[bone_index];
			
			// Put the thumb down on touch
			if (bone_index <= eBone_Thumb3 && thumbTouchPct != 0.0f) {
				// As the models are set up right now the thumb wants to rotate the long way around with glm::mix
				// This uses slerp, which however isn't appropriate for other fingers
				InterpolateBone(bone, bindPose[bone_index], thumbTouchPct, true);
			}
			
			// Curl fingers on trigger touch
			if (triggerTouchPct != 0.0f || triggerPct != 0.0f) {
				// SteamVR does something similar, curling adjacent fingers to make them look more natural
				if (bone_index < eBone_MiddleFinger0 && bone_index >= eBone_IndexFinger0) {
					InterpolateBone(bone, bindPose[bone_index], triggerTouchPct);
				} else if (bone_index < eBone_RingFinger0 && bone_index >= eBone_MiddleFinger0) {
					InterpolateBone(bone, bindPose[bone_index], 0.75f * triggerTouchPct);
				} else if (bone_index < eBone_PinkyFinger0 && bone_index >= eBone_RingFinger0) {
					InterpolateBone(bone, bindPose[bone_index], 0.5f * triggerTouchPct);
				}

			}

			// Bend the index finger on trigger
			if (bone_index < eBone_MiddleFinger0 && bone_index >= eBone_IndexFinger0 && triggerPct > 0.0f) {
				InterpolateBone(bone, squeezePose[bone_index], triggerPct);
			}
			
			// Bend the 3 remaining fingers on grip
			if (bone_index >= eBone_MiddleFinger0 && gripPct > 0.0f) {
				InterpolateBone(bone, squeezePose[bone_index], gripPct);
			}

			return bone;
		});
	};

	auto modelSpaceZDisplace = std::views::transform([](const auto& bone) {
		auto ret = bone;
		ret.position.v[2] += handZDisplacement;
		return ret;
	});

	switch (hand) {
	case ITrackedDevice::HAND_LEFT: {
		switch (transformSpace) {
		case VRSkeletalTransformSpace_Model: {
			std::ranges::copy(
			    boneDataGen(left_hand::bindPoseModelSpace, left_hand::squeezeModelSpace, left_hand::openHandModelSpace)
			        | modelSpaceZDisplace,
			    boneData.begin());
			break;
		}
		case VRSkeletalTransformSpace_Parent: {
			std::ranges::copy(
			    boneDataGen(left_hand::bindPoseParentSpace, left_hand::squeezeParentSpace, left_hand::openHandParentSpace),
			    boneData.begin());
			boneData[eBone_Root].position.v[2] -= handZDisplacement;
			break;
		}
		}
		break;
	}
	case ITrackedDevice::HAND_RIGHT: {
		switch (transformSpace) {
		case VRSkeletalTransformSpace_Model: {
			std::ranges::copy(
			    boneDataGen(right_hand::bindPoseModelSpace, right_hand::squeezeModelSpace, right_hand::openHandModelSpace)
			        | modelSpaceZDisplace,
			    boneData.begin());
			break;
		}
		case VRSkeletalTransformSpace_Parent: {
			std::ranges::copy(
			    boneDataGen(right_hand::bindPoseParentSpace, right_hand::squeezeParentSpace, right_hand::openHandParentSpace),
			    boneData.begin());
			boneData[eBone_Root].position.v[2] -= handZDisplacement;
			break;
		}
		}
		break;
	}
	default: {
		OOVR_LOGF("WARNING: Not a hand: %d", hand);
		return vr::VRInputError_InvalidHandle;
	}
	}

	return vr::VRInputError_None;
}
