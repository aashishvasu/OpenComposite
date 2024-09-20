//
// Created by ZNix on 15/03/2021.
//

#include "stdafx.h"

#include "xrmoreutils.h"
#include <convert.h>

void xr_utils::PoseFromSpace(vr::TrackedDevicePose_t* pose, XrSpace space, vr::ETrackingUniverseOrigin origin,
    std::optional<glm::mat4> extraTransform)
{
	auto baseSpace = xr_space_from_tracking_origin(origin);

	XrSpaceVelocity velocity{ XR_TYPE_SPACE_VELOCITY };
	XrSpaceLocation info{ XR_TYPE_SPACE_LOCATION, &velocity, 0, {} };

	OOVR_FAILED_XR_SOFT_ABORT(xrLocateSpace(space, baseSpace, xr_gbl->GetBestTime(), &info));

	glm::mat4 mat = X2G_om34_pose(info.pose);

	// Apply the extra transform if required - this is applied first, since it's used to swap between the
	// grip and steamvr hand spaces.
	if (extraTransform) {
		mat = mat * extraTransform.value();
	}

	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = (info.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
	pose->mDeviceToAbsoluteTracking = G2S_m34(mat);
	pose->eTrackingResult = pose->bPoseIsValid ? vr::TrackingResult_Running_OK : vr::TrackingResult_Running_OutOfRange;
	pose->vVelocity = X2S_v3f(velocity.linearVelocity); // No offsetting transform - this is in world-space
	pose->vAngularVelocity = X2S_v3f(velocity.angularVelocity); // TODO find out if this needs a transform
}

bool xr_utils::PoseFromHandTracking(vr::TrackedDevicePose_t* pose, XrHandJointLocationsEXT locations, XrHandJointVelocitiesEXT velocities, bool isRight)
{
	const int boneToUse = XR_HAND_JOINT_PALM_EXT;

	XrHandJointLocationEXT location = locations.jointLocations[boneToUse];
	XrHandJointVelocityEXT velocity = velocities.jointVelocities[boneToUse];

	bool positionTracked = location.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT != 0);

	if (!locations.isActive || !positionTracked) {
		return false;
	}

	//glm::mat4 mat = X2G_om34_pose(location.pose);

	XrHandJointLocationEXT indexPosition = locations.jointLocations[XR_HAND_JOINT_RING_PROXIMAL_EXT];
	XrHandJointLocationEXT ringPosition = locations.jointLocations[XR_HAND_JOINT_LITTLE_PROXIMAL_EXT];

	glm::vec3 plus_z = X2G_v3f(ringPosition.pose.position) - X2G_v3f(indexPosition.pose.position);
	glm::vec3 toRotate = {0.0f, isRight ? 1.0f : -1.0f, 0.0f};
	
	glm::quat palmOrientation = X2G_quat(location.pose.orientation);

	glm::vec3 plus_x = palmOrientation * toRotate;

	//in monado this is referred to as "orthonormalize"
	float amnt = glm::dot(plus_x, plus_z) / glm::dot(plus_z, plus_z);
	glm::vec3 projected = plus_z * amnt;
	plus_x = plus_x - projected;

	//normalize plus_x and z
	plus_x = glm::normalize(plus_x);
	plus_z = glm::normalize(plus_z);

	//cross product to get y
	glm::vec3 plus_y = glm::cross(plus_z, plus_x);

	//create a matrix from the vectors

	glm::mat3 m = glm::mat3(
		plus_x.x, plus_y.x, plus_z.x,
		plus_x.y, plus_y.y, plus_z.y,
		plus_x.z, plus_y.z, plus_z.z);

	glm::mat3 m2 = glm::mat3(
		plus_x.x, plus_x.y, plus_x.z,
		plus_y.x, plus_y.y, plus_y.z,
		plus_z.x, plus_z.y, plus_z.z);

	//reverse xyz
	glm::mat3 m3 = glm::mat3(
		plus_z.x, plus_y.x, plus_x.x,
		plus_z.y, plus_y.y, plus_x.y,
		plus_z.z, plus_y.z, plus_x.z);


	glm::mat4 mat = glm::identity<glm::mat4>();
	glm::quat q = glm::quat_cast(m2);
	mat = glm::translate(mat, X2G_v3f(location.pose.position)) * glm::mat4_cast(q);
	
	mat = glm::translate(mat, {isRight ? -0.05f : 0.05f, 0.0f, -0.1f});



	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = true;
	pose->mDeviceToAbsoluteTracking = G2S_m34(mat);
	pose->eTrackingResult = pose->bPoseIsValid ? vr::TrackingResult_Running_OK : vr::TrackingResult_Running_OutOfRange;
	pose->vVelocity = X2S_v3f(velocity.linearVelocity); // No offsetting transform - this is in world-space
	pose->vAngularVelocity = X2S_v3f(velocity.angularVelocity); // TODO find out if this needs a transform

	return true;
}

bool xr_utils::PoseFromHandTrackingWithoutVelocity(vr::TrackedDevicePose_t* pose, const std::vector<XrHandJointLocationEXT>& locations, bool isRight) {
	const int boneToUse = XR_HAND_JOINT_PALM_EXT;

	XrHandJointLocationEXT location = locations[boneToUse];
	//XrHandJointVelocityEXT velocity = velocities.jointVelocities[boneToUse];

	bool positionTracked = location.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT != 0);

	if (!positionTracked) {
		return false;
	}

	//some math taken from Monado's controler simulation to infer a proper grip orientation. This looks "good enough" for now but i'd really like to look at what some other openxr->openvr implementations do.
	//maybe we can learn from them.
	XrHandJointLocationEXT indexPosition = locations[XR_HAND_JOINT_RING_PROXIMAL_EXT];
	XrHandJointLocationEXT ringPosition = locations[XR_HAND_JOINT_LITTLE_PROXIMAL_EXT];

	glm::quat palmOrientation = X2G_quat(location.pose.orientation);

	glm::vec3 plus_z = X2G_v3f(ringPosition.pose.position) - X2G_v3f(indexPosition.pose.position);
	glm::vec3 toRotate = {0.0f, isRight ? 1.0f : -1.0f, 0.0f};
	glm::vec3 plus_x = palmOrientation * toRotate;

	//in monado this is referred to as "orthonormalize"
	float amnt = glm::dot(plus_x, plus_z) / glm::dot(plus_z, plus_z);
	glm::vec3 projected = plus_z * amnt;
	plus_x = plus_x - projected;

	//normalize plus_x and z
	plus_x = glm::normalize(plus_x);
	plus_z = glm::normalize(plus_z);

	//cross product to get y
	glm::vec3 plus_y = glm::cross(plus_z, plus_x);

	//create a matrix from the vectors

	glm::mat3 m2 = glm::mat3(
		plus_x.x, plus_x.y, plus_x.z,
		plus_y.x, plus_y.y, plus_y.z,
		plus_z.x, plus_z.y, plus_z.z);

	//matrix to quaternion
	glm::quat q = glm::quat_cast(m2);

	glm::mat4 mat = glm::identity<glm::mat4>();

	mat = glm::translate(mat, X2G_v3f(location.pose.position)) * glm::mat4_cast(q);

	//weird offset that needs to be hardcoded. messes with the actual hand tracking position so we have to adjust for that. This stuff is weird.
	mat = glm::translate(mat, {isRight ? -0.05f : 0.05f, 0.0f, -0.1f});



	pose->bDeviceIsConnected = true;
	pose->bPoseIsValid = true;
	pose->mDeviceToAbsoluteTracking = G2S_m34(mat);
	pose->eTrackingResult = pose->bPoseIsValid ? vr::TrackingResult_Running_OK : vr::TrackingResult_Running_OutOfRange;
	//pose->vVelocity = X2S_v3f(velocity.linearVelocity); // No offsetting transform - this is in world-space
	//pose->vAngularVelocity = X2S_v3f(velocity.angularVelocity); // TODO find out if this needs a transform
}