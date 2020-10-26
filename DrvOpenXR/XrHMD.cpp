//
// Created by ZNix on 25/10/2020.
//

#include "XrHMD.h"

#include "../OpenOVR/convert.h"

void XrHMD::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
	// TODO supersampling? Is that done here or elsewhere?
	*width = xr_main_view(XruEyeLeft).recommendedImageRectWidth;
	*height = xr_main_view(XruEyeLeft).recommendedImageRectHeight;
}

// from BaseSystem

vr::HmdMatrix44_t XrHMD::GetProjectionMatrix(vr::EVREye eEye, float fNearZ, float fFarZ, EGraphicsAPIConvention convention)
{
	if (convention == API_OpenGL) {
		STUBBED();
	}

	XrViewConfigurationView& eye = xr_main_view((XruEye)eEye);

	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = 0; // TODO FIXME
	locateInfo.space = xr_gbl->floorSpace; // Should make no difference to the FOV

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
	float verticalFov = -tanU + tanD;

	// TODO verify!
	glm::mat4 m;
	m[0] = glm::vec4(twoNear / horizontalFov, 0, (tanL + tanR) / horizontalFov, 0); // First row, determines X out multiplication with vector
	m[1] = glm::vec4(0, twoNear / horizontalFov, (tanU + tanD) / verticalFov, 0); // Determines Y
	m[2] = glm::vec4(0, 0, -(fNearZ + fFarZ) / (fFarZ - fNearZ), (2 * fNearZ * fFarZ) / (fFarZ - fNearZ)); // Determines Z
	m[3] = glm::vec4(0, 0, -1, 0); // Determines W
	m = glm::transpose(m);

	return O2S_m4(m);
}

void XrHMD::GetProjectionRaw(vr::EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
	STUBBED();
}

bool XrHMD::ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t* pDistortionCoordinates)
{
	STUBBED();
}

vr::HmdMatrix34_t XrHMD::GetEyeToHeadTransform(vr::EVREye eEye)
{
	// TODO implement - return an identity matrix for now which completely breaks the stereoscopic effect but lets us move on
	vr::HmdMatrix34_t m{};
	m.m[0][0] = m.m[1][1] = m.m[2][2] = 1;
	return m;
}

bool XrHMD::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	STUBBED();
}

vr::HiddenAreaMesh_t XrHMD::GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType type)
{
	STUBBED();
}
