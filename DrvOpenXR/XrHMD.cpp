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
	XrViewConfigurationView& eye = xr_main_view((XruEye)eEye);

	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = xr_gbl->nextPredictedFrameTime;
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
	float verticalFov = tanU - tanD;

	// To make building these projections easier, we build them as row-major matrices then (for graphics
	// APIs that use column-major matrices) transpose them at the end.

	// TODO verify!
	glm::mat4 m;
	if (convention == API_DirectX) {
		// https://docs.microsoft.com/en-us/windows/win32/dxtecharts/the-direct3d-transformation-pipeline
		// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
		// Also copied from glm::perspectiveLH_ZO
		m[0] = glm::vec4(1 / tanf(horizontalFov / 2), 0, (tanL + tanR) / horizontalFov, 0);
		m[1] = glm::vec4(0, 1 / tanf(verticalFov / 2), (tanU + tanD) / verticalFov, 0);
		m[2] = glm::vec4(0, 0, -fFarZ / (fFarZ - fNearZ), -(fFarZ * fNearZ) / (fFarZ - fNearZ));
		m[3] = glm::vec4(0, 0, -1, 0);
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
	STUBBED();
}

bool XrHMD::ComputeDistortion(vr::EVREye eEye, float fU, float fV, vr::DistortionCoordinates_t* pDistortionCoordinates)
{
	STUBBED();
}

vr::HmdMatrix34_t XrHMD::GetEyeToHeadTransform(vr::EVREye eEye)
{
	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = xr_gbl->nextPredictedFrameTime;
	locateInfo.space = xr_gbl->viewSpace;

	XrViewState state = { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	XrView views[XruEyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };
	OOVR_FAILED_XR_ABORT(xrLocateViews(xr_session, &locateInfo, &state, XruEyeCount, &viewCount, views));
	OOVR_FALSE_ABORT(viewCount == XruEyeCount);

	return G2S_m34(X2G_om34_pose(views[eEye].pose));
}

bool XrHMD::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	STUBBED();
}

vr::HiddenAreaMesh_t XrHMD::GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType type)
{
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

	for (int i = 0; i < mask.indexCountOutput; i++) {
		int index = mask.indices[i];
		XrVector2f v = mask.vertices[index];
		arr[i] = vr::HmdVector2_t{ v.x, v.y };
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
