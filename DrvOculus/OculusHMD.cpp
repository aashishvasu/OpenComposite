#include "DrvOculusCommon.h"
#include "OculusHMD.h"

#include "../OpenOVR/libovr_wrapper.h"
#include "OculusBackend.h"

// TODO fix these
#include "../OpenOVR/stdafx.h"
#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/Reimpl/static_bases.gen.h"
#include "../OpenOVR/convert.h"
#include "../OpenOVR/Misc/Config.h"

#include "Extras/OVR_Math.h"
using namespace OVR;

OculusHMD::OculusHMD(OculusBackend * backend) : OculusDevice(backend) {
}

bool OculusHMD::IsConnected() {
	return true;
}

void OculusHMD::GetRecommendedRenderTargetSize(uint32_t * width, uint32_t * height) {
	ovrSizei size = ovr_GetFovTextureSize(
		*ovr::session,
		ovrEye_Left, // Resolutions are done per-eye in LibOVR, no particular reason for left eye
		ovr::hmdDesc.DefaultEyeFov[ovrEye_Left],
		oovr_global_configuration.SupersampleRatio()
	);

	*width = size.w;
	*height = size.h;
}

ovrPoseStatef OculusHMD::GetOculusPose(const ovrTrackingState & trackingState) {
	return trackingState.HeadPose;
}

ovrPosef OculusHMD::GetOffset() {
	return Posef::Identity();
}

// from BaseSystem

HmdMatrix44_t OculusHMD::GetProjectionMatrix(EVREye eye, float znear, float zfar) {
	return GetProjectionMatrix(eye, znear, zfar, API_DirectX);
}

HmdMatrix44_t OculusHMD::GetProjectionMatrix(EVREye eye, float znear, float zfar, EGraphicsAPIConvention convention) {
	ovrMatrix4f matrix = ovrMatrix4f_Projection(
		ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)],
		znear, zfar,
		convention == API_OpenGL ? ovrProjection_ClipRangeOpenGL : ovrProjection_None // TODO is this right?
	);

	return O2S_m4(matrix);
}

void OculusHMD::GetProjectionRaw(EVREye eye, float * pfLeft, float * pfRight, float * pfTop, float * pfBottom) {
	/**
	* With a straight passthrough:
	*
	* SteamVR Left:  -1.110925, 0.889498, -0.964926, 0.715264
	* SteamVR Right: -1.110925, 0.889498, -0.715264, 0.964926
	* OpenOVR Left:  0.889498, 1.110925, 0.964926, 0.715264
	* OpenOVR Right: 0.889498, 1.110925, 0.715264, 0.964926
	*
	* Via:
	*   char buff[1024];
	*   snprintf(buff, sizeof(buff), "eye=%d %f, %f, %f, %f", eye, *pfTop, *pfBottom, *pfLeft, *pfRight);
	*   OOVR_LOG(buff);
	*
	* This suggests that SteamVR negates the top and left values. We should do that too, for obvious reasons.
	*/

	ovrFovPort fov = ovr::hmdDesc.DefaultEyeFov[S2O_eye(eye)];
	*pfTop = -fov.DownTan; // negate, and for some reason the up and down have to be switched
	*pfBottom = fov.UpTan;
	*pfLeft = -fov.LeftTan; // negate
	*pfRight = fov.RightTan;
}

bool OculusHMD::ComputeDistortion(EVREye eEye, float fU, float fV, DistortionCoordinates_t * out) {
	// Here's what SteamVR does when run on a Rift:
	//  each of the output values match the input values, for all colour channels, regardless of the eye.
	// This is thus essentially a identity transform function.

	if (!out)
		return true;

	out->rfRed[0] = out->rfGreen[0] = out->rfBlue[0] = fU;
	out->rfRed[1] = out->rfGreen[1] = out->rfBlue[1] = fV;

	return true;
}

HmdMatrix34_t OculusHMD::GetEyeToHeadTransform(EVREye ovr_eye) {
	ovrEyeType eye = S2O_eye(ovr_eye);
	ovrPosef &pose = ovr::hmdToEyeViewPose[eye];

	OVR::Matrix4f transform(pose);
	// For some bizzare reason, inverting the matrix (to go from hmd->eye
	// to eye->hmd) breaks the view, and it's fine without it. That or I'm misunderstanding
	// what exactly this method is supposed to return.

	HmdMatrix34_t result;
	O2S_om34(transform, result);
	return result;
}
