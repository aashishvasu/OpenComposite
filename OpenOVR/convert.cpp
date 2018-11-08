#include "stdafx.h"
#include "convert.h"

using namespace vr;

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
