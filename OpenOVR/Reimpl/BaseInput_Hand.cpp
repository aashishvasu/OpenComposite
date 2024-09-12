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

static glm::mat4 readBoneTransform(const vr::VRBoneTransform_t& src)
{
	glm::quat srcQuat;
	quaternionCopy(src.orientation, srcQuat);
	glm::mat4 pose = glm::mat4_cast(srcQuat);
	pose[3] = { src.position.v[0], src.position.v[1], src.position.v[2], 1.f };
	return pose;
}

// Games that use this:
// * NeosVR
// Any others? Please add them to the list!
void BaseInput::ConvertHandModelSpace(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* output)
{
	// The root bone should just be left at identity? TODO check SteamVR
	output[eBone_Root].orientation = vr::HmdQuaternionf_t{ /* w */ 1, 0, 0, 0 };
	output[eBone_Root].position = vr::HmdVector4_t{ 0, 0, 0, 1 };

	// Note: go to https://gltf-viewer.donmccurdy.com/ and load up the hand glTF model from the test suite
	// Turn the axes on and compare it to the OpenXR hand tracking extension diagram:
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_conventions_of_hand_joints

	// We need to apply a local transform to the coordinate spaces of the non-thumb fingers. This is because
	// the bones are set up in such a way that their local-to-the-bone coordinate system that the geometry works
	// in varies between SteamVR and OpenXR. Oddly this is rotated 90deg around the Y axis (local to that bone),
	// which suggests it's not caused by rotating the bone.
	// This has to be the first transform applied when calculating the bone's rotation, since it needs to rotate
	// in the bone's local space rather than in model space.
	glm::mat4 localTransform = glm::rotate(glm::identity<glm::mat4>(), glm::radians(-90.f), { 0, 1, 0 });

	// The wrist bone has it's own special transform, with all axes negated
	glm::mat4 rightWristTransform = glm::zero<glm::mat4>();
	rightWristTransform[1][0] = -1;
	rightWristTransform[0][1] = -1;
	rightWristTransform[2][2] = -1;
	rightWristTransform[3][3] = 1;

	// Wrists are a special case of being different between sides
	glm::mat4 leftWristTransform = glm::zero<glm::mat4>();
	leftWristTransform[1][0] = 1;
	leftWristTransform[0][1] = 1;
	leftWristTransform[2][2] = -1;
	leftWristTransform[3][3] = 1;

	// And the left hand gets it's own special transform
	glm::mat4 leftHandTransform = glm::scale(glm::identity<glm::mat4>(), { -1, -1, 1 });

	for (int XrId = XR_HAND_JOINT_WRIST_EXT; XrId <= XR_HAND_JOINT_LITTLE_TIP_EXT; XrId++) {
		const XrHandJointLocationEXT& this_joint = joints[XrId];
		glm::mat4 pose = X2G_om34_pose(this_joint.pose);

		// Not a bug - xrIds match vrIds except for palm pose and aux bones.
		vr::VRBoneTransform_t& out = output[XrId];

		// Position is easy enough...
		out.position = { pose[3][0], pose[3][1], pose[3][2], 1.f };

		// But the transform to correct the bone's local coordinate system varies between bones, so add
		// that in here. It's also different on the left and right hands.
		if (XrId == XR_HAND_JOINT_WRIST_EXT) {
			if (isRight) {
				pose *= rightWristTransform;
			} else {
				pose *= leftWristTransform;
			}
		} else {
			pose *= localTransform;

			if (!isRight) {
				pose *= leftHandTransform;
			}
		}

		glm::quat out_rotation(pose);
		quaternionCopy(out_rotation, out.orientation);
	}

	OOVR_SOFT_ABORT("Aux bones not yet implemented!");
}

// END MODEL POSE STUFF

// RELATIVE POSE STUFF

// This is just exactly what OC did before. It probably doesn't do the right thing.
void BaseInput::ConvertHandParentSpace(const std::vector<XrHandJointLocationEXT>& joints, bool isRight, VRBoneTransform_t* out_transforms)
{
	// First, get everything into the SteamVR coordinate system, to avoid duplicating the delicate space conversion system
	VRBoneTransform_t modelRelative[31];
	ConvertHandModelSpace(joints, isRight, modelRelative);

	// Load the data into the output bones, with the correct mapping
	std::optional<XrHandJointEXT> parentId;
	auto mapBone = [&](XrHandJointEXT xrId, int vrId) {
		OOVR_FALSE_ABORT(vrId < 31);
		const vr::VRBoneTransform_t& src = modelRelative[xrId];
		vr::VRBoneTransform_t& out = out_transforms[vrId];

		// Read the model-relative transform
		glm::mat4 pose = readBoneTransform(src);

		// All the OpenXR transforms are relative to the space we specified as baseSpace, in this case the grip pose. If the
		// application wants each bone's transform relative to it's parent, apply that now.
		// If this bone is the root bone (parentId is not set), then it's the same in either space mode.
		// Get the required transform from:
		// Tbone_in_model = Tparent_in_model * Tbone_in_parent
		// inv(Tparent_in_model) * Tbone_in_model = Tbone_in_parent
		if (parentId) {
			const vr::VRBoneTransform_t& parent = modelRelative[parentId.value()];
			glm::mat4 parentPose = readBoneTransform(parent);

			pose = glm::affineInverse(parentPose) * pose;
		} else {
			// We're now taking hand data at face value for full fidelity hand tracking (LeapMotion, Mercury, Quest Hand Tracking)
			// and need to apply these to the root bone. I'm honestly unsure *why* it's so horribly offset, including position, but it is what it is.
			// This looks pretty natural on index and quest hand tracking.
			pose = glm::identity<glm::mat4>();

			glm::mat4 handOffsetLeft = glm::rotate(glm::identity<glm::mat4>(), glm::radians(-180.f), { 1, 0, 0 });
			glm::mat4 handOffsetRight = glm::rotate(glm::identity<glm::mat4>(), glm::radians(-180.f), { 1, 0, 0 });

			handOffsetLeft = glm::rotate(handOffsetLeft, glm::radians(-90.f), { 0, 0, 1 });
			handOffsetRight = glm::rotate(handOffsetRight, glm::radians(90.f), { 0, 0, 1 });

			if (isRight) {
				pose *= handOffsetRight;
			} else {
				pose *= handOffsetLeft;
			}
		}

		// TODO eMotionRange, if that's even possible

		glm::quat rotation(pose);
		out.position = vr::HmdVector4_t{ pose[3][0], pose[3][1], pose[3][2], 1.f }; // What's the fourth value for?
		quaternionCopy(rotation, out.orientation);

		// Update the parent to make it convenient to declare bones moving towards the finger tip
		parentId = xrId;
	};

	// Set up the root bone
	out_transforms[0].orientation = vr::HmdQuaternionf_t{ /* w */ 1, 0, 0, 0 };
	out_transforms[0].position = vr::HmdVector4_t{ 0, 0, 0, 1 };

	parentId = {};
	mapBone(XR_HAND_JOINT_WRIST_EXT, eBone_Wrist);

	// clang-format off
	parentId = XR_HAND_JOINT_WRIST_EXT;
	mapBone(XR_HAND_JOINT_THUMB_METACARPAL_EXT, eBone_Thumb0);
	mapBone(XR_HAND_JOINT_THUMB_PROXIMAL_EXT,   eBone_Thumb1);
	mapBone(XR_HAND_JOINT_THUMB_DISTAL_EXT,     eBone_Thumb2);
	mapBone(XR_HAND_JOINT_THUMB_TIP_EXT,        eBone_Thumb3);

	parentId = XR_HAND_JOINT_WRIST_EXT;
	mapBone(XR_HAND_JOINT_INDEX_METACARPAL_EXT,   eBone_IndexFinger0);
	mapBone(XR_HAND_JOINT_INDEX_PROXIMAL_EXT,     eBone_IndexFinger1);
	mapBone(XR_HAND_JOINT_INDEX_INTERMEDIATE_EXT, eBone_IndexFinger2);
	mapBone(XR_HAND_JOINT_INDEX_DISTAL_EXT,       eBone_IndexFinger3);
	mapBone(XR_HAND_JOINT_INDEX_TIP_EXT,          eBone_IndexFinger4);

	parentId = XR_HAND_JOINT_WRIST_EXT;
	mapBone(XR_HAND_JOINT_MIDDLE_METACARPAL_EXT,   eBone_MiddleFinger0);
	mapBone(XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT,     eBone_MiddleFinger1);
	mapBone(XR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT, eBone_MiddleFinger2);
	mapBone(XR_HAND_JOINT_MIDDLE_DISTAL_EXT,       eBone_MiddleFinger3);
	mapBone(XR_HAND_JOINT_MIDDLE_TIP_EXT,          eBone_MiddleFinger4);

	parentId = XR_HAND_JOINT_WRIST_EXT;
	mapBone(XR_HAND_JOINT_RING_METACARPAL_EXT,   eBone_RingFinger0);
	mapBone(XR_HAND_JOINT_RING_PROXIMAL_EXT,     eBone_RingFinger1);
	mapBone(XR_HAND_JOINT_RING_INTERMEDIATE_EXT, eBone_RingFinger2);
	mapBone(XR_HAND_JOINT_RING_DISTAL_EXT,       eBone_RingFinger3);
	mapBone(XR_HAND_JOINT_RING_TIP_EXT,          eBone_RingFinger4);

	parentId = XR_HAND_JOINT_WRIST_EXT;
	mapBone(XR_HAND_JOINT_LITTLE_METACARPAL_EXT,   eBone_PinkyFinger0);
	mapBone(XR_HAND_JOINT_LITTLE_PROXIMAL_EXT,     eBone_PinkyFinger1);
	mapBone(XR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT, eBone_PinkyFinger2);
	mapBone(XR_HAND_JOINT_LITTLE_DISTAL_EXT,       eBone_PinkyFinger3);
	mapBone(XR_HAND_JOINT_LITTLE_TIP_EXT,          BaseInput::eBone_PinkyFinger4);
	// clang-format on

	// TODO aux bones - they're equal to the distal bones but always use VRSkeletalTransformSpace_Model mode
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
