#include "stdafx.h"

#include "convert.h"

#include <glm/gtx/matrix_decompose.hpp>

using namespace vr;

using Vec3 = MfVector3f;
using Mat4 = MfMatrix4f;

HmdMatrix44_t O2S_m4(Mat4 input)
{
	HmdMatrix44_t output;

	// Note: both value_ptr and HmdMatrix44_t are column-major
	// See https://github.com/ValveSoftware/openvr/issues/433
	memcpy_s(output.m, sizeof(float[4][4]), glm::value_ptr(input), sizeof(float[4][4]));

	return output;
}

XruEye S2O_eye(EVREye eye)
{
	return eye == Eye_Left ? XruEyeLeft : XruEyeRight;
}

void O2S_v3f(const Vec3& in, vr::HmdVector3_t& out)
{
	out.v[0] = in.x;
	out.v[1] = in.y;
	out.v[2] = in.z;
}

void O2S_om34(const Mat4& in, HmdMatrix34_t& out)
{
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 4; x++) {
			out.m[y][x] = in[y][x];
		}
	}
}

void S2O_om44(const HmdMatrix34_t& in, MfMatrix4f& out)
{
	out = glm::identity<MfMatrix4f>();
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 4; x++) {
			out[y][x] = in.m[y][x];
		}
	}
}

static float getScaleForCol(const Mat4& m, int col)
{
	return glm::length(glm::vec3(m[0][col], m[1][col], m[2][col]));
}

static void multScaleForCol(Mat4& m, int col, float mul)
{
	m[0][col] *= mul;
	m[1][col] *= mul;
	m[2][col] *= mul;
}

XrPosef S2O_om34_pose(const vr::HmdMatrix34_t& in)
{
	Mat4 otm;
	S2O_om44(in, otm);

	// Extract scaling from the matrix
	// If a scaled quaternion is passed into the Quatf constructor, it explodes
	// Thus we need to remove the scaling
	// See https://math.stackexchange.com/a/1463487 for how this works
	// Translation is (M[0][3], M[1][3], M[2][3])

	float scaleX = getScaleForCol(otm, 0);
	float scaleY = getScaleForCol(otm, 1);
	float scaleZ = getScaleForCol(otm, 2);

	multScaleForCol(otm, 0, 1 / scaleX);
	multScaleForCol(otm, 1, 1 / scaleY);
	multScaleForCol(otm, 2, 1 / scaleZ);

	// TODO use this to modify the scaling for the panel in the world?
	// More testing is needed to see how this works

	// Things we care about
	glm::vec3 translation;
	glm::quat rotation;

	// Rubbish
	glm::vec3 scale, skew;
	glm::vec4 perspective;

	// Break down our matrix into a translation and rotation
	glm::decompose(otm, scale, rotation, translation, skew, perspective);

	XrPosef pose = { 0 };
	pose.position = G2X_v3f(translation);
	pose.orientation = G2X_quat(rotation);

	return pose;
}

XrVector3f G2X_v3f(const glm::vec3& vec)
{
	return XrVector3f{ vec.x, vec.y, vec.z };
}

XrQuaternionf G2X_quat(const glm::quat& q)
{
	return XrQuaternionf{ q.x, q.y, q.z, q.w };
}

vr::HmdVector3_t X2S_v3f(const XrVector3f& vec)
{
	HmdVector3_t out = {};
	out.v[0] = vec.x;
	out.v[1] = vec.y;
	out.v[2] = vec.z;
	return out;
}

vr::HmdVector3_t G2S_v3f(const glm::vec3& vec)
{
    return HmdVector3_t{ vec.x, vec.y, vec.z };
}
