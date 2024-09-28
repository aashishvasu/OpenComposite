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

static void quaternionInverse(XrQuaternionf* result, const XrQuaternionf& src)
{
	result->x = -src.x;
	result->y = -src.y;
	result->z = -src.z;
	result->w = src.w;
}

static void quaternionMultiply(XrQuaternionf* dst, const XrQuaternionf& a, const XrQuaternionf& b)
{
	dst->x = (b.w * a.x) + (b.x * a.w) + (b.y * a.z) - (b.z * a.y);
	dst->y = (b.w * a.y) - (b.x * a.z) + (b.y * a.w) + (b.z * a.x);
	dst->z = (b.w * a.z) + (b.x * a.y) - (b.y * a.x) + (b.z * a.w);
	dst->w = (b.w * a.w) - (b.x * a.x) - (b.y * a.y) - (b.z * a.z);
}

static XrQuaternionf ApplyBoneHandTransform(XrQuaternionf quat, bool isRight)
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

static void vectorSubtract(XrVector3f* dst, XrVector3f a, XrVector3f b)
{
	dst->x = a.x - b.x;
	dst->y = a.y - b.y;
	dst->z = a.z - b.z;
}

static float vectorLength(const XrVector3f& a)
{
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

static void vectorRotate(XrVector3f* dst, const XrQuaternionf& a, const XrVector3f& v)
{
	XrQuaternionf q = { v.x, v.y, v.z, 0.f };
	XrQuaternionf aq;
	quaternionMultiply(&aq, q, a);
	XrQuaternionf aInv;
	quaternionInverse(&aInv, a);
	XrQuaternionf aqaInv;
	quaternionMultiply(&aqaInv, aInv, aq);

	dst->x = aqaInv.x;
	dst->y = aqaInv.y;
	dst->z = aqaInv.z;
}

static glm::mat4 readBoneTransform(const vr::VRBoneTransform_t& src)
{
	glm::quat srcQuat;
	quaternionCopy(src.orientation, srcQuat);
	glm::mat4 pose = glm::mat4_cast(srcQuat);
	pose[3] = { src.position.v[0], src.position.v[1], src.position.v[2], 1.f };
	return pose;
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

static bool GetJointRelative(const XrHandJointLocationEXT& currentJoint, const XrHandJointLocationEXT& parentJoint, bool isRight, vr::VRBoneTransform_t& outBoneTransform)
{
	XrQuaternionf quatInvParent;
	quaternionInverse(&quatInvParent, parentJoint.pose.orientation);
	XrQuaternionf quatDiffXR;
	quaternionMultiply(&quatDiffXR, currentJoint.pose.orientation, quatInvParent);

	XrQuaternionf quatDiffVR = ApplyBoneHandTransform(quatDiffXR, isRight);
	quaternionCopy(quatDiffVR, outBoneTransform.orientation);

	XrVector3f vecDiffFromParent;
	vectorSubtract(&vecDiffFromParent, currentJoint.pose.position, parentJoint.pose.position);

	float boneLength = vectorLength(vecDiffFromParent);
	outBoneTransform.position = { boneLength, 0.f, 0.f, 1 };

	if (isRight) {
		outBoneTransform.position.v[0] *= -1;
	}

	return true;
}

static vr::VRBoneTransform_t leftOpenPose[2] = {
	{ { 0.000000f, 0.000000f, 0.000000f, 1.000000f }, { 1.000000f, 0.000000f, 0.000000f, 0.000000f } },
	{ { -0.034038f, 0.036503f, 0.164722f, 1.000000f }, { -0.055147f, -0.078608f, -0.920279f, 0.379296f } },
};

static vr::VRBoneTransform_t rightOpenPose[2] = {
	{ { 0.000000f, 0.000000f, 0.000000f, 1.000000f }, { 1.000000f, -0.000000f, -0.000000f, 0.000000f } },
	{ { 0.034038f, 0.036503f, 0.164722f, 1.000000f }, { -0.055147f, -0.078608f, 0.920279f, -0.379296f } },
};

static bool MetacarpalJointPass(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output, OOVR_EVRSkeletalTransformSpace space)
{
	for (int joint :
	    {
	        XR_HAND_JOINT_THUMB_METACARPAL_EXT,
	        XR_HAND_JOINT_INDEX_METACARPAL_EXT,
	        XR_HAND_JOINT_MIDDLE_METACARPAL_EXT,
	        XR_HAND_JOINT_RING_METACARPAL_EXT,
	        XR_HAND_JOINT_LITTLE_METACARPAL_EXT }) {

		const XrHandJointLocationEXT& currentJoint = joints[joint];
		const XrHandJointLocationEXT& parentJoint = joints[XR_HAND_JOINT_WRIST_EXT];

		if (currentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0) || parentJoint.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT == 0))
			return false;

		XrQuaternionf quatInvParent;
		quaternionInverse(&quatInvParent, parentJoint.pose.orientation);
		XrQuaternionf quatDiffXR;
		quaternionMultiply(&quatDiffXR, currentJoint.pose.orientation, quatInvParent);

		XrQuaternionf quatDiffVR = ApplyBoneHandTransform(quatDiffXR, isRight);

		XrQuaternionf quatMagic;
		quatMagic.w = 0.5f;
		quatMagic.x = 0.5f;
		quatMagic.y = -0.5f;
		quatMagic.z = 0.5f;

		if (isRight) {
			quatMagic.x *= -1;
			quatMagic.y *= -1;
		}

		XrQuaternionf quatFinalDiff;
		quaternionMultiply(&quatFinalDiff, quatDiffVR, quatMagic);

		quaternionCopy(quatFinalDiff, output[joint].orientation);

		XrVector3f vecDiffFromParent;
		vectorSubtract(&vecDiffFromParent, currentJoint.pose.position, parentJoint.pose.position);

		XrVector3f vecWristRel;
		vectorRotate(&vecWristRel, quatInvParent, vecDiffFromParent);

		output[joint].position.v[0] = vecWristRel.y;
		output[joint].position.v[1] = vecWristRel.x;
		output[joint].position.v[2] = -vecWristRel.z;
		output[joint].position.v[3] = 1.f;

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
// Logic will be adjusted to glm.
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
