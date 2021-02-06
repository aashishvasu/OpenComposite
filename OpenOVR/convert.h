#pragma once
#include "Misc/xrutil.h"

// TODO clean up this mess

vr::HmdMatrix44_t O2S_m4(MfMatrix4f input);
XruEye S2O_eye(vr::EVREye eye); // TODO inline without link errors
void O2S_v3f(const MfVector3f& in, vr::HmdVector3_t& out);
void O2S_om34(const MfMatrix4f& in, vr::HmdMatrix34_t& out);

void S2O_om44(const vr::HmdMatrix34_t& in, MfMatrix4f& out);
XrPosef S2O_om34_pose(const vr::HmdMatrix34_t& in);

XrVector3f G2X_v3f(const glm::vec3& vec);
XrQuaternionf G2X_quat(const glm::quat& q);

glm::vec3 X2G_v3f(const XrVector3f& q);
glm::quat X2G_quat(const XrQuaternionf& q);
glm::mat4 X2G_om34_pose(const XrPosef& in);

vr::HmdVector3_t G2S_v3f(const glm::vec3& vec);
vr::HmdMatrix34_t G2S_m34(const glm::mat4& mat);

vr::HmdVector3_t X2S_v3f(const XrVector3f& vec);
