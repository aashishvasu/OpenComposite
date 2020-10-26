//
// Created by ZNix on 25/10/2020.
//

#pragma once

#ifdef SUPPORT_DX
#include <d3d11.h>
#define XR_USE_GRAPHICS_API_D3D11
#endif

#include <openxr/openxr_platform.h>

#include <vector>

/**
 * A wrapper class for the function pointers to OpenXR extensions.
 *
 * Modern versions of the OpenXR SDK don't provide implementations of the extension
 * methods, and we have to use xrGetInstanceProcAddr to get said function pointers
 * manually. One global instance of XrExt is defined in xrutil.h as xr_ext which is
 * setup when the OpenXR instance is created.
 *
 * See https://github.com/microsoft/OpenXR-MixedReality/issues/32
 */
class XrExt {
public:
	XrExt();

	PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
};

// Put this here rather tha xrutil.h so not all files have to include vector
extern std::vector<XrViewConfigurationView> xr_views_list;
