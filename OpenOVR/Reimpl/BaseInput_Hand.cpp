#include "Misc/xrmoreutils.h"
#include "generated/interfaces/public_vrtypes.h"
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

// https://github.com/ValveSoftware/openvr/blob/master/samples/drivers/utils/vrmath/vrmath.h
static HmdQuaternionf_t operator*(const vr::HmdQuaternionf_t& lhs, const vr::HmdQuaternionf_t& rhs)
{
	return {
		lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z,
		lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
		lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
	};
}

static HmdVector4_t operator*(const HmdVector4_t& a, const HmdVector4_t& b)
{
	return {
		a.v[0] * b.v[0],
		a.v[1] * b.v[1],
		a.v[2] * b.v[2],
		1.f
	};
}

static VRBoneTransform_t operator*(const VRBoneTransform_t& a, const VRBoneTransform_t& b)
{
	return {
		a.position * b.position,
		a.orientation * b.orientation,
	};
}

static bool isBoneMetacarpal(XrHandJointEXT handJoint)
{
	return handJoint == XR_HAND_JOINT_THUMB_METACARPAL_EXT || handJoint == XR_HAND_JOINT_INDEX_METACARPAL_EXT || handJoint == XR_HAND_JOINT_MIDDLE_METACARPAL_EXT || handJoint == XR_HAND_JOINT_RING_METACARPAL_EXT || handJoint == XR_HAND_JOINT_LITTLE_METACARPAL_EXT;
}

// converts a range of finger bones to model space.
static void convertJointRange(HandSkeletonBone start, HandSkeletonBone end, VRBoneTransform_t* joints)
{
	HandSkeletonBone parentId = eBone_Wrist;

	for (int i = start; i <= end; i++) {
		joints[i] = joints[parentId] * joints[i];
		parentId = (HandSkeletonBone)i;
	}
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

static void InterpolateBone(VRBoneTransform_t& bone, const VRBoneTransform_t& targetBone, float pct, bool slerp = false)
{
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

static auto GetInterpolatedControllerState(const ITrackedDevice::TrackedDeviceType hand, const LegacyControllerActions controller) {
	struct ControllerState {
		float triggerPct;
		float gripPct;
		float thumbTouchPct;
		float triggerTouchPct;
	} output;

	XrActionStateGetInfo info{
		.type = XR_TYPE_ACTION_STATE_GET_INFO,
		.action = controller.trigger,
		.subactionPath = XR_NULL_PATH,
	};

	XrActionStateFloat state{ XR_TYPE_ACTION_STATE_FLOAT };
	XrActionStateBoolean stateBool{ XR_TYPE_ACTION_STATE_BOOLEAN };

	constexpr float simulateCurlThreshold = 0.08f;

	// Trigger State
	XrResult actionRes = xrGetActionStateFloat(xr_session.get(), &info, &state);
	output.triggerPct = 0.0f;
	if (XR_SUCCEEDED(actionRes) && state.currentState >= simulateCurlThreshold) {
		output.triggerPct = state.currentState;
	} else if (XR_FAILED(actionRes)) {
		OOVR_LOGF("WARNING: couldn't get trigger percentage (%d)", actionRes);
	}

	// Grip State
	info.action = controller.grip;
	actionRes = xrGetActionStateFloat(xr_session.get(), &info, &state);
	output.gripPct = 0.0f;
	if (XR_SUCCEEDED(actionRes) && state.currentState >= simulateCurlThreshold) {
		output.gripPct = state.currentState;
	} else if (XR_FAILED(actionRes)) {
		OOVR_LOGF("WARNING: couldn't get grip percentage (%d)", actionRes);
	}

	// Trigger Touch State
	info.action = controller.triggerTouch;
	actionRes = xrGetActionStateBoolean(xr_session.get(), &info, &stateBool);
	bool triggerTouch = XR_SUCCEEDED(actionRes) && stateBool.currentState;

	// Force touch to true when trigger is being pressed
	if (output.triggerPct != 0.0f)
		triggerTouch = true;


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
	if (!hasThumbInput) {
		thumbTouch = output.gripPct != 1.0f || output.triggerPct == 1.0f;
	}

	static std::array<std::chrono::high_resolution_clock::time_point, 2> lastTime{
		std::chrono::high_resolution_clock::now(),
		std::chrono::high_resolution_clock::now()
	};

	// Calculate per-hand deltaTime for binary input interpolation
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTimeDuration = currentTime - lastTime[hand];
	float deltaTime = deltaTimeDuration.count();
	lastTime[hand] = currentTime;

	// Interpolated states
	static std::array<float, 2> thumbTouchPcts = {0.0f, 0.0f};
	static std::array<float, 2> triggerTouchPcts = {0.0f, 0.0f};

	constexpr float touchTransitionSpeed = 8.0f;

	// Update interpolated values
	thumbTouchPcts[hand] = glm::clamp(thumbTouchPcts[hand] + (thumbTouch ? deltaTime * touchTransitionSpeed : -deltaTime * touchTransitionSpeed), 0.0f, 1.0f);
	triggerTouchPcts[hand] = glm::clamp(triggerTouchPcts[hand] + (triggerTouch ? deltaTime * touchTransitionSpeed : -deltaTime * touchTransitionSpeed), 0.0f, 1.0f);

	output.thumbTouchPct = thumbTouchPcts[hand];
	output.triggerTouchPct = triggerTouchPcts[hand];

	return output;
}

void BaseInput::ApplyHandOffset(ITrackedDevice::TrackedDeviceType hand, VRBoneTransform_t* boneData) {
	// Controller -> hand root offsets (thanks danwillm)
	constexpr float handXOffset = -0.04; // This value taken from ALVR actually, no idea why it's the correct one
	constexpr float handYOffset = 0.02;
	constexpr float handZOffset = 0.15;
	constexpr float handAngleOffset = 45;

	// Note to self and others, never transform the root bone, it will doom you

	VRBoneTransform_t *boneRoot = &boneData[eBone_Wrist];

	boneRoot->position.v[0] += hand == ITrackedDevice::HAND_LEFT ? handXOffset : -handXOffset;
	boneRoot->position.v[1] += handYOffset;
	boneRoot->position.v[2] += handZOffset;

	glm::quat rootOrientation(boneRoot->orientation.w, boneRoot->orientation.x, boneRoot->orientation.y, boneRoot->orientation.z);
	rootOrientation = glm::rotate(rootOrientation, glm::radians(handAngleOffset), glm::vec3(1.0f, 0.0f, 0.0f));

	boneRoot->orientation.x = rootOrientation.x;
	boneRoot->orientation.y = rootOrientation.y;
	boneRoot->orientation.z = rootOrientation.z;
	boneRoot->orientation.w = rootOrientation.w;
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

void BaseInput::ParentSpaceSkeletonToModelSpace(VRBoneTransform_t* joints)
{
	// To convert joints in parent space to model space, we need to concatenate each bone going up the chain.
	// Using the index finger as an example:
	//
	// it has 5 bones. IndexFinger0 - IndexFinger4.
	// for bone 0, we multiply it by the wrist.
	// for bone 1, we multiply it by the result of 0, etc.
	//
	// Aux bones are all just multiplied with the wrist.
	VRBoneTransform_t wrist = joints[eBone_Wrist];

	convertJointRange(eBone_Thumb0, eBone_Thumb3, joints);
	convertJointRange(eBone_IndexFinger0, eBone_IndexFinger4, joints);
	convertJointRange(eBone_MiddleFinger0, eBone_MiddleFinger4, joints);
	convertJointRange(eBone_RingFinger0, eBone_RingFinger4, joints);
	convertJointRange(eBone_PinkyFinger0, eBone_PinkyFinger4, joints);

	for (int i = eBone_Aux_Thumb; i <= eBone_Aux_PinkyFinger; i++) {
		joints[i] = wrist * joints[i];
	}
}

EVRInputError BaseInput::getEstimatedBoneData(
    ITrackedDevice::TrackedDeviceType hand,
    EVRSkeletalTransformSpace transformSpace,
    std::span<VRBoneTransform_t, eBone_Count> boneData)
{
	auto& controller = legacyControllers[hand];

	auto state = GetInterpolatedControllerState(hand, controller);

	auto boneDataGen = [state](const BoneArray& bindPose, const BoneArray& squeezePose, const BoneArray& openHandPose) {
		return std::ranges::iota_view(0, static_cast<int>(eBone_Count)) | std::views::transform([=](int bone_index) {
			auto bone = openHandPose[bone_index];

			// Put the thumb down on touch
			if (state.thumbTouchPct != 0.0f) {
				if ((bone_index >= eBone_Thumb0 && bone_index <= eBone_Thumb3) || bone_index == eBone_Aux_Thumb) 
					InterpolateBone(bone, bindPose[bone_index], state.thumbTouchPct);
			}

			// Curl fingers on trigger touch
			if (state.triggerTouchPct != 0.0f || state.triggerPct != 0.0f) {
				// SteamVR does something similar, curling adjacent fingers to make them look more natural
				if ((bone_index >= eBone_IndexFinger0 && bone_index <= eBone_IndexFinger4) || bone_index == eBone_Aux_IndexFinger)
					InterpolateBone(bone, squeezePose[bone_index], 0.4f * state.triggerTouchPct);
				else if ((bone_index >= eBone_MiddleFinger0 && bone_index <= eBone_MiddleFinger4) || bone_index == eBone_Aux_MiddleFinger)
					InterpolateBone(bone, squeezePose[bone_index], 0.2f * state.triggerTouchPct);
				else if ((bone_index >= eBone_RingFinger0 && bone_index <= eBone_RingFinger4) || bone_index == eBone_Aux_RingFinger)
					InterpolateBone(bone, squeezePose[bone_index], 0.1f * state.triggerTouchPct);
			}

			// Bend the index finger on trigger
			if (state.triggerPct != 0.0f) {
				if ((bone_index >= eBone_IndexFinger0 && bone_index <= eBone_IndexFinger4) || bone_index == eBone_Aux_IndexFinger)
					InterpolateBone(bone, squeezePose[bone_index], state.triggerPct);
			}

			// Bend the 3 remaining fingers on grip
			if (state.gripPct != 0.0f) {
				if ((bone_index >= eBone_MiddleFinger0 && bone_index <= eBone_PinkyFinger4) || (bone_index >= eBone_Aux_MiddleFinger && bone_index <= eBone_Aux_PinkyFinger))
					InterpolateBone(bone, squeezePose[bone_index], state.gripPct);
			}

			return bone;
		});
	};

	switch (hand) {
	case ITrackedDevice::HAND_LEFT: {
			std::ranges::copy(
			    boneDataGen(left_hand::bindPoseParentSpace, left_hand::squeezeParentSpace, left_hand::openHandParentSpace),
			    boneData.begin());
		break;
	}
	case ITrackedDevice::HAND_RIGHT: {
			std::ranges::copy(
			    boneDataGen(right_hand::bindPoseParentSpace, right_hand::squeezeParentSpace, right_hand::openHandParentSpace),
			    boneData.begin());
		break;
	}
	default: {
		OOVR_LOGF("WARNING: Not a hand: %d", hand);
		return vr::VRInputError_InvalidHandle;
	}
	}

	// Apply offsets to position the hand correctly
	ApplyHandOffset(hand, boneData.data());

	if (transformSpace == EVRSkeletalTransformSpace::VRSkeletalTransformSpace_Model)
		ParentSpaceSkeletonToModelSpace(boneData.data());

	return vr::VRInputError_None;
}

EVRInputError BaseInput::getEstimatedSkeletalSummary(ITrackedDevice::TrackedDeviceType hand, VRSkeletalSummaryData_t* pSkeletalSummaryData)
{
	OOVR_FALSE_ABORT(hand != ITrackedDevice::HAND_NONE);

	std::fill(std::begin(pSkeletalSummaryData->flFingerSplay), std::end(pSkeletalSummaryData->flFingerSplay), 0.2f);

	LegacyControllerActions& controller = legacyControllers[hand];

	auto state = GetInterpolatedControllerState(hand, controller);

	// Replicate what getEstimatedBoneData is doing

	// Curl fingers on trigger touch
	pSkeletalSummaryData->flFingerCurl[VRFinger_Thumb] = state.thumbTouchPct;
	pSkeletalSummaryData->flFingerCurl[VRFinger_Index] = 0.4f * state.triggerTouchPct;
	pSkeletalSummaryData->flFingerCurl[VRFinger_Middle] = 0.2f * state.triggerTouchPct;
	pSkeletalSummaryData->flFingerCurl[VRFinger_Ring] = 0.1f * state.triggerTouchPct;
	pSkeletalSummaryData->flFingerCurl[VRFinger_Pinky] = 0.0f;

	// Curl index finger fully if triggered
	if (state.triggerPct != 0.0f) {
		pSkeletalSummaryData->flFingerCurl[VRFinger_Index] = glm::max(pSkeletalSummaryData->flFingerCurl[VRFinger_Index], state.triggerPct);
	}

	// Curl remaining fingers based on grip percentage
	if (state.gripPct != 0.0f) {
		pSkeletalSummaryData->flFingerCurl[VRFinger_Middle] = glm::max(pSkeletalSummaryData->flFingerCurl[VRFinger_Middle], state.gripPct);
		pSkeletalSummaryData->flFingerCurl[VRFinger_Ring] = glm::max(pSkeletalSummaryData->flFingerCurl[VRFinger_Ring], state.gripPct);
		pSkeletalSummaryData->flFingerCurl[VRFinger_Pinky] = glm::max(pSkeletalSummaryData->flFingerCurl[VRFinger_Pinky], state.gripPct);
	}

	return vr::VRInputError_None;
}
