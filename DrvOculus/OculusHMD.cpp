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

OculusHMD::~OculusHMD() {
	for (auto &hiddenAreaMesh : hiddenAreaMeshes) {
		delete[] hiddenAreaMesh.pVertexData;
	}
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

bool OculusHMD::GetTimeSinceLastVsync(float * pfSecondsSinceLastVsync, uint64_t * pulFrameCounter) {
	// Bit of a mess
	// See https://github.com/ValveSoftware/openvr/issues/518
	// And https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetDeviceToAbsoluteTrackingPose

	float fDisplayFrequency = GetFloatTrackedDeviceProperty(vr::Prop_DisplayFrequency_Float, nullptr);
	float fFrameDuration = 1.f / fDisplayFrequency;
	float fVsyncToPhotons = GetFloatTrackedDeviceProperty(vr::Prop_SecondsFromVsyncToPhotons_Float, nullptr);

	// Here's how a game would calculate the timing:
	// float timeUntilNextVsync = fFrameDuration - fSecondsSinceLastVsync;
	// float fPredictedSecondsFromNow = timeUntilNextVsync + fVsyncToPhotons;
	// Since we want fPredictedSecondsFromNow to equal the predicted display time, reverse this to find it:

	float desiredPhotonTime = ovr_GetPredictedDisplayTime(*ovr::session, 0);
	float desiredPhotonTimeFromNow = desiredPhotonTime - ovr_GetTimeInSeconds();

	float timeSinceStartOfFrame = fFrameDuration - desiredPhotonTimeFromNow;
	float fFakeSecondsSinceLastVsync = timeSinceStartOfFrame + fVsyncToPhotons;

	if (pfSecondsSinceLastVsync)
		*pfSecondsSinceLastVsync = fFakeSecondsSinceLastVsync;

	return true;
}

OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetViewportStencil(
		ovrSession session,
		const ovrViewportStencilDesc* viewportStencilDesc,
		ovrViewportStencilMeshBuffer* outMeshBuffer);

HiddenAreaMesh_t OculusHMD::GetHiddenAreaMesh(EVREye eEye, EHiddenAreaMeshType type) {
	// TODO should we not cache this?
	if (hiddenAreaMeshes[eEye].pVertexData) {
		return hiddenAreaMeshes[eEye];
	}

	if (!oovr_global_configuration.UseViewportStencil()) {
		HiddenAreaMesh_t &result = hiddenAreaMeshes[eEye];
		result.pVertexData = NULL;
		result.unTriangleCount = 0;
		return result;
	}

	ovrEyeType eye = eEye == Eye_Left ? ovrEye_Left : ovrEye_Right;
	ovrViewportStencilDesc desc;
	desc.Eye = eye;
	desc.FovPort = ovr::hmdDesc.DefaultEyeFov[eye];
	desc.HmdToEyeRotation = ovr::eyeRenderDesc[eye].HmdToEyePose.Orientation;

	if (type == k_eHiddenAreaMesh_Inverse) {
		desc.StencilType = ovrViewportStencil_VisibleArea;
	}
	else if (type == k_eHiddenAreaMesh_LineLoop) {
		desc.StencilType = ovrViewportStencil_BorderLine;
	}
	else {
		desc.StencilType = ovrViewportStencil_HiddenArea;
	}

	ovrViewportStencilMeshBuffer mb = { 0 };
	mb.AllocVertexCount = 0;
	mb.VertexBuffer = NULL;
	mb.AllocIndexCount = 0;
	mb.IndexBuffer = NULL;

	// Query for the size
	ovr_GetViewportStencil(*ovr::session, &desc, &mb);

	// Create the buffers
	mb.AllocVertexCount = mb.UsedVertexCount;
	mb.VertexBuffer = new ovrVector2f[mb.AllocVertexCount];
	mb.AllocIndexCount = mb.UsedIndexCount;
	mb.IndexBuffer = new uint16_t[mb.AllocIndexCount];

	// Get the data
	ovr_GetViewportStencil(*ovr::session, &desc, &mb);

	// Convert the data into something usable by SteamVR
	HiddenAreaMesh_t &result = hiddenAreaMeshes[eEye];
	vr::HmdVector2_t *arr = new vr::HmdVector2_t[mb.UsedIndexCount];
	result.pVertexData = arr;

	for (int i = 0; i < mb.UsedIndexCount; i++) {
		int index = mb.IndexBuffer[i];
		ovrVector2f &v = mb.VertexBuffer[index];

		arr[i] = HmdVector2_t{v.x, v.y};
	}

	if (type == k_eHiddenAreaMesh_LineLoop) {
		result.unTriangleCount = mb.UsedIndexCount;
	}
	else {
		result.unTriangleCount = mb.UsedIndexCount / 3;
	}

	// Delete the buffers
	delete mb.VertexBuffer;
	delete mb.IndexBuffer;

	// Return the result
	return result;
}

// Properties
bool OculusHMD::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case Prop_DeviceProvidesBatteryStatus_Bool:
		return false;
	case Prop_HasDriverDirectModeComponent_Bool:
		return true; // Who knows what this is used for? It's in HL:A anyway.
	}

	return OculusDevice::GetBoolTrackedDeviceProperty(prop, pErrorL);
}

float OculusHMD::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case Prop_DisplayFrequency_Float:
		return 90.0; // TODO grab this from LibOVR
	case Prop_LensCenterLeftU_Float:
	case Prop_LensCenterLeftV_Float:
	case Prop_LensCenterRightU_Float:
	case Prop_LensCenterRightV_Float:
		// SteamVR reports it as unknown
		if (pErrorL)
			*pErrorL = TrackedProp_UnknownProperty;
		return 0;
	case Prop_UserIpdMeters_Float:
		return BaseSystem::SGetIpd();
	case Prop_SecondsFromVsyncToPhotons_Float:
		// Seems to be used by croteam games, IDK what the real value is, 100s should do
		return 0.0001f;
	case Prop_UserHeadToEyeDepthMeters_Float:
		// TODO ensure this has the correct sign, though it seems to always be zero anyway
		// In any case, see: https://github.com/ValveSoftware/openvr/issues/398
		return ovr::hmdToEyeViewPose[ovrEye_Left].Position.z;
	}

	return OculusDevice::GetInt32TrackedDeviceProperty(prop, pErrorL);
}

uint64_t OculusHMD::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pErrorL) {
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	// no HMD properties here for now

	return OculusDevice::GetUint64TrackedDeviceProperty(prop, pErrorL);
}

uint32_t OculusHMD::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
	char * value, uint32_t bufferSize, vr::ETrackedPropertyError * pErrorL) {

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

#define PROP(in, out) \
if(prop == in) { \
	if (value != NULL && bufferSize > 0) { \
		strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
	} \
	return (uint32_t) strlen(out) + 1; \
}

	// Only CV1 has been validated
	switch (ovr::hmdDesc.Type) {
	case ovrHmd_DK1:
		PROP(Prop_ModelNumber_String, "Oculus Rift DK1");
		break;
	case ovrHmd_DK2:
		PROP(Prop_ModelNumber_String, "Oculus Rift DK2");
		break;
	case ovrHmd_CV1:
		PROP(Prop_ModelNumber_String, "Oculus Rift CV1");
		PROP(Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F");
		break;
	default:
		PROP(Prop_ModelNumber_String, "<unknown>");
		break;
	}

	PROP(Prop_RenderModelName_String, "oculusHmdRenderModel");

	return OculusDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}
