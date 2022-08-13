#include "stdafx.h"
#define BASE_IMPL
#include "BaseInput.h"

#include <convert.h>

#include <glm/ext.hpp>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>

#include <optional>

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

static void transformJointModelSpace(const std::vector<XrHandJointLocationEXT>& joints, VRBoneTransform_t* out_transforms, XrHandJointEXT xrId, bool is_right)
{
	const XrHandJointLocationEXT& this_joint = joints[xrId];
	const XrHandJointLocationEXT& parent_joint = joints[0];
	glm::mat4 joint_global_pose = X2G_om34_pose(this_joint.pose);

	glm::vec3 tr = { joint_global_pose[3][0], joint_global_pose[3][1], joint_global_pose[3][2] };

	// Not a bug - xrIds match vrIds except for palm pose and aux bones.
	vr::VRBoneTransform_t& out = out_transforms[xrId];

	// Almost definitely wrong. Neos doesn't care about translations at all, so I can't test this.
	out.position.v[0] = tr.y;
	out.position.v[1] = is_right ? -tr.x : tr.x;
	out.position.v[2] = -tr.z;
	out.position.v[3] = 1.0f;

	glm::mat3x3 mat(joint_global_pose);

	// glm is column major.
	// But I could be wrong documenting this here aaaaaa
	// Fuck OpenVR.
	// [ 0  0 -1]
	// [ 0 -1  0]
	// [-1  0  0]
	glm::mat3x3 mat_mul_to_proposed_left_hand = { { 0, 0, -1 }, { 0, -1, 0 }, { -1, 0, 0 } };
	// [ 0  0 -1]
	// [ 0  1  0]
	// [ 1  0  0]
	glm::mat3x3 mat_mul_to_proposed_right_hand = { { 0, 0, 1 }, { 0, 1, 0 }, { -1, 0, 0 } };

	glm::mat3x3 permuted_mat;
	permuted_mat = mat * (is_right ? mat_mul_to_proposed_right_hand : mat_mul_to_proposed_left_hand);

	// Also, the above matrix multiplication is just swapping and/or negating some rows of the 3x3 rotation matrix.
	// You could accomplish this slightly faster without a matrix multiplication
	// or possibly by smartly applying a quaternion rotation then swapping/negating some of the quat elements?
	// Either way this seems clearest and we're not running on a Quest 2 so eh

	glm::quat out_rotation(permuted_mat);

	quaternionCopy(out_rotation, out.orientation);
}

// Games that use this:
// * NeosVR
// Any others? Please add them to the list!
void BaseInput::ConvertHandModelSpace(const std::vector<XrHandJointLocationEXT>& joints, const bool is_right, VRBoneTransform_t* out_transforms)
{
	// Root seems to be totally 100% ignored by Neos. Let's set it to something not-insane.
	out_transforms[eBone_Root].orientation = vr::HmdQuaternionf_t{ /* w */ 1, 0, 0, 0 };
	out_transforms[eBone_Root].position = vr::HmdVector4_t{ 0, 0, 0, 1 };

	// Set the wrist bone position
	// Yep, that's right, we literally just copy the position of the wrist from OpenXR into the wrist in OpenVR.
	// I am beyond shocked that no axis flips were required considering the rest of my experience.
	out_transforms[eBone_Wrist].position.v[0] = joints[XR_HAND_JOINT_WRIST_EXT].pose.position.x;
	out_transforms[eBone_Wrist].position.v[1] = joints[XR_HAND_JOINT_WRIST_EXT].pose.position.y;
	out_transforms[eBone_Wrist].position.v[2] = joints[XR_HAND_JOINT_WRIST_EXT].pose.position.z;
	out_transforms[eBone_Wrist].position.v[3] = 1.0f;

	// Calculate the wrist bone orientation
	// Note: This post-rotation stuff might be slightly wrong. It works with Index knuckles hand emulation and Index camera optical hand tracking, at least.
	glm::quat wrist_rot;
	quaternionCopy(joints[XR_HAND_JOINT_WRIST_EXT].pose.orientation, wrist_rot);

#if 0
	// These paths should do the same thing.
	glm::mat4 PostRotate_0_Rotate180X = glm::rotate(glm::identity<glm::mat4>(), math_pi, glm::vec3(1, 0, 0));

	// Note: this has to be the opposite between L and R. For now we only care about L, sooo
	// -math_pi / 2.0f for L, M_PIf / 2.0f for R
	glm::mat4 PostRotate_0_Rotate90Z = glm::rotate(glm::identity<glm::mat4>(), is_right ? math_pi / 2.0f : -M_PIf / 2.0f, glm::vec3(0, 0, 1));

	glm::quat q(PostRotate_0_Rotate180X * PostRotate_0_Rotate90Z);
	OOVR_LOGF("postrotate %f %f %f %f", q.w, q.x, q.y, q.z);
#else
	glm::quat q;
	q.w = 0;
	q.x = is_right ? -0.707107 : 0.707107;
	q.y = 0.707107;
	q.z = 0;
#endif

	glm::quat outRotation = wrist_rot * q;
	quaternionCopy(outRotation, out_transforms[eBone_Wrist].orientation);

	for (int XrId = XR_HAND_JOINT_THUMB_METACARPAL_EXT; XrId <= XR_HAND_JOINT_LITTLE_TIP_EXT; XrId++) {
		transformJointModelSpace(joints, out_transforms, (XrHandJointEXT)XrId, is_right);
	}
	// Todo: aux bones. Neos doesn't use them sooo

	return;
}

// END MODEL POSE STUFF

// RELATIVE POSE STUFF

// This is just exactly what OC did before. It probably doesn't do the right thing.
void BaseInput::ConvertHandParentSpace(const std::vector<XrHandJointLocationEXT>& joints, const bool is_right, VRBoneTransform_t* out_transforms)
{
	// Annoyingly the coordinate system between this extension and OpenVR is also different. As per the wiki page:
	// https://github.com/ValveSoftware/openvr/wiki/Hand-Skeleton
	// The hands effectively sit on their sides in the bind pose. The right hand's palm faces -X and the left hand's
	// palm faces +X. In the OpenXR extension, the hands are as they would be if you placed them on a table: palms on
	// both hands are down (-Y) and the tops of your hands are up (+Y). Therefore the normal of your thumbnails are
	// facing towards the opposite hand. Fortunately Z represents the same axis in both coordinate spaces.
	// Therefore roll the right-hand clockwise 90deg around Z and the left hand counter-clockwise around Z (careful with
	// what direction Z is if you're trying to visualise this).
	float angle_mult;
	if (!is_right) {
		angle_mult = 1.0f; // Natural rotation is CCW, invert for clockwise
	} else {
		angle_mult = -1.0f;
	}
	glm::mat4 systemTransform = glm::rotate(glm::identity<glm::mat4>(), angle_mult * math_pi / 2.0f, glm::vec3(0, 0, 1));

	// Load the data into the output bones, with the correct mapping
	std::optional<XrHandJointEXT> parentId;
	auto mapBone = [&](XrHandJointEXT xrId, int vrId) {
		const XrHandJointLocationEXT& src = joints.at(xrId);
		OOVR_FALSE_ABORT(vrId < 31);
		vr::VRBoneTransform_t& out = out_transforms[vrId];

		// Read the OpenXR transform
		// I don't think there's anything we can do if the validity flags are false, so just ignore them
		glm::mat4 pose = systemTransform * X2G_om34_pose(src.pose);

		// All the OpenXR transforms are relative to the space we specified as baseSpace, in this case the grip pose. If the
		// application wants each bone's transform relative to it's parent, apply that now.
		// If this bone is the root bone (parentId is not set), then it's the same in either space mode.
		// Get the required transform from:
		// Tbone_in_model = Tparent_in_model * Tbone_in_parent
		// inv(Tparent_in_model) * Tbone_in_model = Tbone_in_parent
		if (parentId) {
			const XrHandJointLocationEXT& parent = joints.at(parentId.value());
			glm::mat4 parentPose = systemTransform * X2G_om34_pose(parent.pose);

			pose = glm::affineInverse(parentPose) * pose;
		}

		// TODO eMotionRange, if that's even possible

		glm::quat rotation(pose);
		out.position = vr::HmdVector4_t{ pose[3][0], pose[3][1], pose[3][2], 1.f }; // What's the fourth value for?
		out.orientation = vr::HmdQuaternionf_t{ rotation.w, rotation.x, rotation.y, rotation.z };

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
