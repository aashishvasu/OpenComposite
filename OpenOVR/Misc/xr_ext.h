//
// Created by ZNix on 25/10/2020.
//

#pragma once

// TODO Turtle1331 it would be nicer to include OpenXR-SDK's xr_dependencies.h if possible
#ifdef SUPPORT_DX
#include <d3d11.h>
#define XR_USE_GRAPHICS_API_D3D11
#endif

#ifdef SUPPORT_GL
#include <GL/gl.h>
#define XR_USE_GRAPHICS_API_OPENGL
#endif

#ifdef SUPPORT_VK
#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#endif

#ifdef _WIN32
#define XR_OS_WINDOWS
#define XR_USE_PLATFORM_WIN32
#endif

#ifdef ANDROID
#include <jni.h>
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

#ifdef SUPPORT_DX
	PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
#endif
#ifdef SUPPORT_VK
	PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR = nullptr;
	PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR = nullptr;
	PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR = nullptr;
	PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR = nullptr;
#endif
#ifdef SUPPORT_GL
	PFN_xrGetOpenGLGraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR = nullptr;
#endif

	PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR = nullptr;
};

// Put this here rather tha xrutil.h so not all files have to include vector
extern std::vector<XrViewConfigurationView> xr_views_list;
