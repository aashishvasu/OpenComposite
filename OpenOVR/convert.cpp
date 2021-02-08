#include "stdafx.h"

#include "convert.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace vr;

using Vec3 = MfVector3f;
using Mat4 = MfMatrix4f;

HmdMatrix44_t O2S_m4(Mat4 input)
{
	HmdMatrix44_t output{};

	// Note: while value_ptr is column-major, HmdMatrix44_t is (sometimes?) row-major
	// This issue claiming HmdMatrix is column-major wasted quite a bit of my time: https://github.com/ValveSoftware/openvr/issues/433
	// In hindsight HmdMatrix_34 should have been enough of a hint since it has the position data on the bottom
	// However for the 4x4 matrix it's not obvious which way around it is - pass it through plain and let this function's user
	// figure out what's correct based on the context.

	memcpy(output.m, glm::value_ptr(input), sizeof(float[4][4]));

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

glm::vec3 X2G_v3f(const XrVector3f& v)
{
	return glm::vec3(v.x, v.y, v.z);
}

glm::quat X2G_quat(const XrQuaternionf& q)
{
	return glm::quat(q.w, q.x, q.y, q.z);
}

glm::mat4 X2G_om34_pose(const XrPosef& in)
{
	// WARNING: glm::translate does not translate a matrix! It builds a transform matrix you can then use
	// to translate something else.
	return glm::translate(glm::mat4(1.0f), X2G_v3f(in.position)) * glm::toMat4(X2G_quat(in.orientation));
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

vr::HmdMatrix34_t G2S_m34(const glm::mat4& mat)
{
	HmdMatrix34_t out = {};

	// TODO compile with Clang since afaik MSVC doesn't have an unroll pragma and manual unrolling is ugly
	// Note that HmdMatrix34_t is (obviously) row-major (otherwise it wouldn't store a transform) while GLM
	// uses column-major matrices. Thus we transpose it during the loop.
#pragma unroll
	for (int x = 0; x < 3; x++) {
#pragma unroll
		for (int y = 0; y < 4; y++) {
			out.m[x][y] = mat[y][x];
		}
	}
	return out;
}
