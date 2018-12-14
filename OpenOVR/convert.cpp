#include "stdafx.h"
#include "convert.h"

using namespace vr;

using Vec3 = OVR::Vector3f;
using Mat4 = OVR::Matrix4f;

HmdMatrix44_t O2S_m4(ovrMatrix4f input) {
	HmdMatrix44_t output;

	memcpy_s(output.m, sizeof(float[4][4]), input.M, sizeof(float[4][4]));

	return output;
}

ovrEyeType S2O_eye(EVREye eye) {
	return eye == Eye_Left ? ovrEye_Left : ovrEye_Right;
}

void O2S_v3f(const ovrVector3f & in, vr::HmdVector3_t & out) {
	out.v[0] = in.x;
	out.v[1] = in.y;
	out.v[2] = in.z;
}

void O2S_om34(const OVR::Matrix4f & in, HmdMatrix34_t & out) {
	for (size_t y = 0; y < 3; y++) {
		for (size_t x = 0; x < 4; x++) {
			out.m[y][x] = in.M[y][x];
		}
	}
}

void S2O_om44(const HmdMatrix34_t & in, OVR::Matrix4f & out) {
	out = OVR::Matrix4f::Identity();
	for (size_t y = 0; y < 3; y++) {
		for (size_t x = 0; x < 4; x++) {
			out.M[y][x] = in.m[y][x];
		}
	}
}

static float getScaleForCol(const Mat4 &m, int col) {
	return Vec3(m.M[0][col], m.M[1][col], m.M[2][col]).Length();
}

static void multScaleForCol(Mat4 &m, int col, float mul) {
	m.M[0][col] *= mul;
	m.M[1][col] *= mul;
	m.M[2][col] *= mul;
}

ovrPosef S2O_om34_pose(const vr::HmdMatrix34_t & in) {
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

	ovrPosef pose = { 0 };
	pose.Position = otm.GetTranslation();
	pose.Orientation = OVR::Quatf(otm);

	return pose;
}
