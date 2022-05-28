//
// Created by ZNix on 25/10/2020.
//

#include "XrHMD.h"

#include "../OpenOVR/Misc/Config.h"
#include "../OpenOVR/Misc/xrmoreutils.h"
#include "../OpenOVR/Reimpl/BaseSystem.h"
#include "../OpenOVR/convert.h"

void XrHMD::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
	*width = (uint32_t)((float)xr_main_view(XruEyeLeft).recommendedImageRectWidth * oovr_global_configuration.SupersampleRatio());
	*height = (uint32_t)((float)xr_main_view(XruEyeLeft).recommendedImageRectHeight * oovr_global_configuration.SupersampleRatio());
}

// from BaseSystem

vr::HmdMatrix44_t XrHMD::GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ, EGraphicsAPIConvention convention)
{
	if (eEye < 0 || (int)eEye >= 2)
		eEye = vr::Eye_Left;

	XrViewConfigurationView& eye = xr_main_view((XruEye)eEye);

	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = xr_gbl->GetBestTime();

	// Seated space is available first when starting up, depending on the runtime implementation. We quickly get accurate
	// values from xrLocateViews, but that doesn't help if the app only calls GetProjectionMatrix once and stores the
	// bad values.
	// TODO A better way to solve this would be to submit a few blank frames when we're using the temporary device to
	// let this value settle, along with any other similar data.
	locateInfo.space = xr_gbl->seatedSpace; // Should make no difference to the FOV

	XrViewState state = { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	XrView views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };
	OOVR_FAILED_XR_ABORT(xrLocateViews(xr_session, &locateInfo, &state, XruEyeCount, &viewCount, views));
	OOVR_FALSE_ABORT(viewCount == XruEyeCount);

	// Build the projection matrix
	// It looks like there aren't any functions in glm that can take different l/r/t/b FOV values, so do it ourselves
	// Also calculate the projection matrix as row major (so one column determines one value when multiplied with a
	// vector) and then transpose it back to being a column-major vector.
	XrFovf& fov = views[eEye].fov;

	float twoNear = fNearZ * 2;
	float tanL = tanf(fov.angleLeft);
	float tanR = tanf(fov.angleRight);
	float tanU = tanf(fov.angleUp);
	float tanD = tanf(fov.angleDown);
	float horizontalFov = -tanL + tanR;
	float verticalFov = tanU - tanD;

	// To make building these projections easier, we build them as row-major matrices then (for graphics
	// APIs that use column-major matrices) transpose them at the end if needed.

	// TODO verify!
	glm::mat4 m;
	if (convention == API_DirectX) {
		// https://docs.microsoft.com/en-us/windows/win32/dxtecharts/the-direct3d-transformation-pipeline
		// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
		// Also copied from glm::perspectiveLH_ZO
		m[0] = glm::vec4(2 / horizontalFov, 0, (tanL + tanR) / horizontalFov, 0);
		m[1] = glm::vec4(0, 2 / verticalFov, (tanU + tanD) / verticalFov, 0);
		m[2] = glm::vec4(0, 0, -fFarZ / (fFarZ - fNearZ), -(fFarZ * fNearZ) / (fFarZ - fNearZ));
		m[3] = glm::vec4(0, 0, -1, 0);

		// Don't transpose, since D3D uses row-major matricies anyway.
	} else {
		// FIXME this transform is almost certainly wrong
		m[0] = glm::vec4(twoNear / horizontalFov, 0, (tanL + tanR) / horizontalFov, 0); // First row, determines X out multiplication with vector
		m[1] = glm::vec4(0, twoNear / horizontalFov, (tanU + tanD) / verticalFov, 0); // Determines Y
		m[2] = glm::vec4(0, 0, -(fNearZ + fFarZ) / (fFarZ - fNearZ), -(2 * fNearZ * fFarZ) / (fFarZ - fNearZ)); // Determines Z
		m[3] = glm::vec4(0, 0, -1, 0); // Determines W

		m = glm::transpose(m);

		STUBBED(); // TODO make sure the OpenGL transform matrix is correct
	}

	return O2S_m4(m);
}

void XrHMD::GetProjectionRaw(vr::EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
	// This is how SteamVR seems to handle invalid eyes
	if (eEye < 0 || (int)eEye >= 2)
		eEye = vr::Eye_Left;

	// TODO deduplicate with GetProjectionMatrix
	XrViewConfigurationView& eye = xr_main_view((XruEye)eEye);

	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = xr_gbl->GetBestTime();

	// Seated space is available first when starting up
	locateInfo.space = xr_gbl->seatedSpace; // Should make no difference to the FOV

	XrViewState state = { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	XrView views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };
	OOVR_FAILED_XR_SOFT_ABORT(xrLocateViews(xr_session, &locateInfo, &state, XruEyeCount, &viewCount, views));
	OOVR_FALSE_ABORT(viewCount == XruEyeCount);

	XrFovf& fov = views[eEye].fov;

	/**
	 * With a straight passthrough:
	 *
	 * SteamVR Left:  -1.110925, 0.889498, -0.964926, 0.715264
	 * SteamVR Right: -1.110925, 0.889498, -0.715264, 0.964926
	 *
	 * For the Rift S:
	 * OpenXR Left: 1.000000, -1.150368, -0.965689, 1.035530
	 * SteamVR Left: -1.150368, 1.000000, -0.965689, 1.035530
	 *
	 * Via:
	 *   char buff[1024];
	 *   snprintf(buff, sizeof(buff), "eye=%d %f, %f, %f, %f", eye, *pfTop, *pfBottom, *pfLeft, *pfRight);
	 *   OOVR_LOG(buff);
	 *
	 * This suggests that SteamVR negates the top and left values, which obviously we need to match. OpenXR
	 * also negates the bottom and left value. Since it appears that SteamVR flips the top and bottom angles, we
	 * can just do that and it'll match.
	 */

	// Unfortunately this can occur on WMR, from the data not being valid very early on. See the comments in
	// GetProjectionMatrix for a further discussion of this.
	if (fov.angleDown == 0.0f && fov.angleUp == 0.0f)
		OOVR_LOG("Warning! FOV is 0");

	*pfTop = tanf(fov.angleDown);
	*pfBottom = tanf(fov.angleUp);
	*pfLeft = tanf(fov.angleLeft);
	*pfRight = tanf(fov.angleRight);
}

bool XrHMD::ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t* pDistortionCoordinates)
{
	STUBBED();
}

vr::HmdMatrix34_t XrHMD::GetEyeToHeadTransform(vr::EVREye eEye)
{
	static XrTime time = ~0; // Don't set to zero by default, otherwise we'll return an identity matrix before the first frame
	static XrView views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };

	// This won't exactly work for HMDs designed for spiders, but it's how SteamVR handles invalid eye numbers.
	if (eEye < 0 || (int)eEye >= 2)
		eEye = vr::Eye_Left;

	if (time != xr_gbl->GetBestTime()) {
		XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
		locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
		locateInfo.displayTime = xr_gbl->GetBestTime();
		locateInfo.space = xr_gbl->viewSpace;

		uint32_t viewCount = 0;
		XrViewState viewState = { XR_TYPE_VIEW_STATE };
		XrView _views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };
		OOVR_FAILED_XR_SOFT_ABORT(xrLocateViews(xr_session, &locateInfo, &viewState, XruEyeCount, &viewCount, _views));

		if (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT && viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) {
			OOVR_FALSE_ABORT(viewCount == XruEyeCount);
			views[0] = _views[0];
			views[1] = _views[1];
			time = xr_gbl->GetBestTime();
		}
	}

	return G2S_m34(X2G_om34_pose(views[eEye].pose));
}

bool XrHMD::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	// TODO: Use frame time statistics to give proper data
	OOVR_LOG_ONCE("Warning: static value returned");
	if (pfSecondsSinceLastVsync)
		*pfSecondsSinceLastVsync = 0.011f;

	return false;
}

vr::HiddenAreaMesh_t XrHMD::GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType type)
{
	if (!xr_ext->xrGetVisibilityMaskKHR_Available()) {
		// This is what the docs say we should return if the mask is unavailable
		return vr::HiddenAreaMesh_t{ nullptr, 0 };
	}

	// TODO verify the line loop mode works properly

	XrVisibilityMaskTypeKHR xrType;
	switch (type) {
	case vr::k_eHiddenAreaMesh_Standard:
		xrType = XR_VISIBILITY_MASK_TYPE_HIDDEN_TRIANGLE_MESH_KHR;
		break;
	case vr::k_eHiddenAreaMesh_Inverse:
		xrType = XR_VISIBILITY_MASK_TYPE_VISIBLE_TRIANGLE_MESH_KHR;
		break;
	case vr::k_eHiddenAreaMesh_LineLoop:
		xrType = XR_VISIBILITY_MASK_TYPE_LINE_LOOP_KHR;
		break;
	default:
		OOVR_ABORTF("Invalid vr::EHiddenAreaMeshType value %d", type);
	}

	// Note: the OpenXR and OpenVR eye indexes are the same, so we can just cast between them.
	auto eye = (uint32_t)eEye;

	// First find out what size we need to allocate
	// Note that this doesn't return XR_ERROR_SIZE_INSUFFICIENT, since setting one of the input counts to zero is special-cased
	XrVisibilityMaskKHR mask = { XR_TYPE_VISIBILITY_MASK_KHR };
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVisibilityMaskKHR(xr_session, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, eye, xrType, &mask));

	// TODO handle cases where a mask isn't available

	// Allocate memory for the mesh
	mask.vertexCapacityInput = mask.vertexCountOutput;
	mask.indexCapacityInput = mask.indexCountOutput;
	mask.indices = new uint32_t[mask.indexCapacityInput];
	mask.vertices = new XrVector2f[mask.vertexCapacityInput];

	// Now actually request the mask data
	OOVR_FAILED_XR_ABORT(xr_ext->xrGetVisibilityMaskKHR(xr_session, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, eye, xrType, &mask));

	// Convert the data into something usable by SteamVR - it doesn't use indices
	vr::HiddenAreaMesh_t result = {};
	// FIXME memory allocation handling
	auto* arr = new vr::HmdVector2_t[mask.indexCountOutput];
	result.pVertexData = arr;

	float ftop, fbottom, fleft, fright;
	GetProjectionRaw(eEye, &fleft, &fright, &ftop, &fbottom);

	for (uint32_t i = 0; i < mask.indexCountOutput; i++) {
		int index = mask.indices[i];
		XrVector2f v = mask.vertices[index];

		if (oovr_global_configuration.EnableHiddenMeshFix()) {
			if (fabs(v.y - ftop) > 0.001 && fabs(v.y - fbottom) > 0.001 ) {
				arr[i] = vr::HmdVector2_t{ (v.x - fleft) / (fright - fleft), (v.y * oovr_global_configuration.HiddenMeshVerticalScale() - ftop) / (fbottom - ftop) };
			} else {
				arr[i] = vr::HmdVector2_t{ (v.x - fleft) / (fright - fleft), (v.y - ftop) / (fbottom - ftop) };
			}
		} else {
			arr[i] = vr::HmdVector2_t{ v.x, v.y };
		}
	}

	if (type == vr::k_eHiddenAreaMesh_LineLoop) {
		result.unTriangleCount = mask.indexCountOutput;
	} else {
		result.unTriangleCount = mask.indexCountOutput / 3;
	}

	// Delete the buffers
	delete[] mask.indices;
	delete[] mask.vertices;

	return result;
}

void XrHMD::GetPose(vr::ETrackingUniverseOrigin origin, vr::TrackedDevicePose_t* pose, ETrackingStateType trackingState)
{
	// TODO use ETrackingStateType

	xr_utils::PoseFromSpace(pose, xr_gbl->viewSpace, origin);
}

float XrHMD::GetIPD()
{
	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = xr_gbl->GetBestTime();
	locateInfo.space = xr_gbl->viewSpace;

	XrViewState state = { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	XrView views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };

	OOVR_FAILED_XR_SOFT_ABORT(xrLocateViews(xr_session, &locateInfo, &state, XruEyeCount, &viewCount, views));
	OOVR_FALSE_ABORT(viewCount == XruEyeCount);

	static float ipd = 0.0064;

	if (state.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT && state.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT)
		ipd = views[vr::Eye_Right].pose.position.x - views[vr::Eye_Left].pose.position.x;

	return ipd;
}

// Properties
bool XrHMD::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case vr::Prop_DeviceProvidesBatteryStatus_Bool:
		return false;
	case vr::Prop_HasDriverDirectModeComponent_Bool:
		return true; // Who knows what this is used for? It's in HL:A anyway.
	case vr::Prop_ContainsProximitySensor_Bool:
		return true;
	case vr::Prop_HasCameraComponent_Bool:
		return false;
	case vr::Prop_HasDisplayComponent_Bool:
		return true;
	case vr::Prop_HasVirtualDisplayComponent_Bool:
		return false;
	default:
		break;
	}

	return XrTrackedDevice::GetBoolTrackedDeviceProperty(prop, pErrorL);
}

float XrHMD::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case vr::Prop_DisplayFrequency_Float:
		return 90.0; // TODO use the real value
	case vr::Prop_LensCenterLeftU_Float:
	case vr::Prop_LensCenterLeftV_Float:
	case vr::Prop_LensCenterRightU_Float:
	case vr::Prop_LensCenterRightV_Float:
		// SteamVR reports it as unknown
		if (pErrorL)
			*pErrorL = vr::TrackedProp_UnknownProperty;
		return 0;
	case vr::Prop_UserIpdMeters_Float:
		return BaseSystem::SGetIpd();
	case vr::Prop_SecondsFromVsyncToPhotons_Float:
		// Seems to be used by croteam games, IDK what the real value is, 100us should do
		return 0.0001f;
	case vr::Prop_UserHeadToEyeDepthMeters_Float:
		// TODO ensure this has the correct sign, though it seems to always be zero anyway
		// In any case, see: https://github.com/ValveSoftware/openvr/issues/398
#ifdef OC_XR_PORT
		XR_STUBBED();
#else
		return ovr::hmdToEyeViewPose[ovrEye_Left].Position.z;
#endif
	default:
		break;
	}

	return XrTrackedDevice::GetInt32TrackedDeviceProperty(prop, pErrorL);
}

uint32_t XrHMD::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop,
    char* value, uint32_t bufferSize, vr::ETrackedPropertyError* pErrorL)
{

	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

#define PROP(in, out)                                                                                      \
	do {                                                                                                   \
		if (prop == in) {                                                                                  \
			if (value != NULL && bufferSize > 0) {                                                         \
				strcpy_s(value, bufferSize, out); /* FFS msvc - strncpy IS the secure version of strcpy */ \
			}                                                                                              \
			return (uint32_t)strlen(out) + 1;                                                              \
		}                                                                                                  \
	} while (0)

	// Pretend everything is a Quest 2 (because that's what I have the property JSON of in the openvr-tests repo)
	PROP(vr::Prop_ModelNumber_String, "Oculus Quest2");
	PROP(vr::Prop_RegisteredDeviceType_String, "oculus/F00BAAF00F");
	PROP(vr::Prop_ControllerType_String, "rift");
	PROP(vr::Prop_ExpectedControllerType_String, "oculus_touch");

	PROP(vr::Prop_RenderModelName_String, "oculusHmdRenderModel");

	return XrTrackedDevice::GetStringTrackedDeviceProperty(prop, value, bufferSize, pErrorL);
}

int32_t XrHMD::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pErrorL)
{
	if (pErrorL)
		*pErrorL = vr::TrackedProp_Success;

	switch (prop) {
	case vr::Prop_DeviceClass_Int32:
		return vr::TrackedDeviceClass_HMD;
	case vr::Prop_ExpectedControllerCount_Int32:
		return 2;
	case vr::Prop_ExpectedTrackingReferenceCount_Int32:
		return 0;
	default:
		break;
	}

	return ITrackedDevice::GetInt32TrackedDeviceProperty(prop, pErrorL);
}
