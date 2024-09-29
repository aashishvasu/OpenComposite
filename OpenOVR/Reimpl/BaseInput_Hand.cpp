#include "Misc/xrmoreutils.h"
#include "stdafx.h"
#define GLM_ENABLE_EXPERIMENTAL
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

static bool MetacarpalJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output, OOVR_EVRSkeletalTransformSpace space)
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
		glm::quat magicRotation = glm::quat(0.5f, isRight ? -0.5f : 0.5, isRight ? 0.5f : -0.5f, 0.5f);
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

static bool FlexionJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output, OOVR_EVRSkeletalTransformSpace space)
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

static bool AuxJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output, OOVR_EVRSkeletalTransformSpace space)
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

// OpenXR Hand Joints to OpenVR Hand Skeleton logic generously donated by danwillm from valve.
bool BaseInput::XrHandJointsToSkeleton(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output, OOVR_EVRSkeletalTransformSpace space)
{
	for (int i : { XR_HAND_JOINT_PALM_EXT, XR_HAND_JOINT_WRIST_EXT }) {
		output[i] = isRight ? rightOpenPose[i] : leftOpenPose[i];
	}

	if (!MetacarpalJointPass(joints, isRight, output, space))
		return false;

	if (!FlexionJointPass(joints, isRight, output, space))
		return false;

	if (!AuxJointPass(joints, isRight, output, space))
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
	XrResult actionRes = xrGetActionStateFloat(xr_session.get(), &info, &state);

	float trigger_pct = 0.0f;
	if (XR_SUCCEEDED(actionRes) && state.currentState >= simulateCurlThreshold) {
		trigger_pct = state.currentState;
	} else if (XR_FAILED(actionRes)) {
		OOVR_LOGF("WARNING: couldn't get trigger percentage (%d)", actionRes);
	}

	info.action = controller.grip;
	actionRes = xrGetActionStateFloat(xr_session.get(), &info, &state);
	float grip_pct = 0.0f;
	if (XR_SUCCEEDED(actionRes) && state.currentState >= simulateCurlThreshold) {
		grip_pct = state.currentState;
	} else if (XR_FAILED(actionRes)) {
		OOVR_LOGF("WARNING: couldn't get grip percentage (%d)", actionRes);
	}

	auto boneDataGen = [trigger_pct, grip_pct](const BoneArray& bindPose, const BoneArray& squeezePose, const BoneArray& openHandPose) {
		return std::ranges::iota_view(0, static_cast<int>(eBone_Count)) | std::views::transform([=](int bone_index) {
			auto bone = bindPose[bone_index];
			// set pointer and thumb straight on grip
			if (bone_index <= eBone_IndexFinger4 && grip_pct > 0.0f && trigger_pct == 0.0f) {
				bone = openHandPose[bone_index];
			}

			// set all fingers to fist on trigger (and last 3 fingers on grip)
			float bend_pct = (bone_index > eBone_IndexFinger4 && grip_pct > 0.0f) ? grip_pct : trigger_pct;
			if (bend_pct > 0.0f) {
				const glm::vec3 startPos(bone.position.v[0], bone.position.v[1], bone.position.v[2]);
				const glm::quat startRot(bone.orientation.w, bone.orientation.x, bone.orientation.y, bone.orientation.z);

				const auto squeezeBone = squeezePose[bone_index];
				const glm::vec3 fistPos(squeezeBone.position.v[0], squeezeBone.position.v[1], squeezeBone.position.v[2]);
				const glm::quat fistRot(squeezeBone.orientation.w, squeezeBone.orientation.x, squeezeBone.orientation.y, squeezeBone.orientation.z);

				// interpolate between bind pose and fist
				const glm::vec3 resPos = glm::mix(startPos, fistPos, bend_pct);
				const glm::quat resRot = glm::mix(startRot, fistRot, bend_pct);
				bone.position = { resPos.x, resPos.y, resPos.z, 1.f };
				bone.orientation = { resRot.w, resRot.x, resRot.y, resRot.z };
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
