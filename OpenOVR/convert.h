#pragma once
#include "OVR_CAPI.h"
#include "Extras/OVR_Math.h"

vr::HmdMatrix44_t O2S_m4(ovrMatrix4f input);
ovrEyeType S2O_eye(vr::EVREye eye); // TODO inline without link errors
void O2S_v3f(const ovrVector3f &in, vr::HmdVector3_t &out);
void O2S_om34(const OVR::Matrix4f &in, vr::HmdMatrix34_t &out);

void S2O_om44(const vr::HmdMatrix34_t & in, OVR::Matrix4f & out);
