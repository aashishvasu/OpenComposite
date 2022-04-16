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

#ifndef _WIN32
// Currently just support XLIB
#define XR_USE_PLATFORM_XLIB
#include <GL/glx.h>
//#define XR_USE_PLATFORM_XCB
#endif

#endif

#ifdef SUPPORT_GLES
#include <EGL/egl.h>
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

#include "../logging.h"

#include <openxr/openxr_platform.h>

#include <vector>

typedef uint32_t XrGraphicsApiSupportedFlags;

// Flag bits for supported graphics apis
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHCIS_API_D3D11 = 0x0001;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHCIS_API_D3D12 = 0x0002;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHCIS_API_GL    = 0x0004;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHCIS_API_GLES  = 0x0008;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHCIS_API_VK    = 0x0010;

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
	XrExt(XrGraphicsApiSupportedFlags apis);

#ifdef SUPPORT_DX
	XrResult xrGetD3D11GraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsD3D11KHR* graphicsRequirements){
		OOVR_FALSE_ABORT(pXrGetD3D11GraphicsRequirementsKHR);
		return pXrGetD3D11GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#ifdef SUPPORT_VK
	XrResult xrGetVulkanInstanceExtensionsKHR(XrInstance instance, XrSystemId systemId, uint32_t bufferCapacityInput, uint32_t* bufferCountOutput, char* buffer){
		OOVR_FALSE_ABORT(pXrGetVulkanInstanceExtensionsKHR);
		return pXrGetVulkanInstanceExtensionsKHR(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer);
	}
	XrResult xrGetVulkanDeviceExtensionsKHR( XrInstance instance, XrSystemId systemId, uint32_t bufferCapacityInput, uint32_t* bufferCountOutput, char* buffer){
		OOVR_FALSE_ABORT(pXrGetVulkanDeviceExtensionsKHR);
		return pXrGetVulkanDeviceExtensionsKHR( instance, systemId, bufferCapacityInput, bufferCountOutput, buffer);
	}
	XrResult xrGetVulkanGraphicsDeviceKHR(XrInstance instance, XrSystemId systemId, VkInstance vkInstance, VkPhysicalDevice* vkPhysicalDevice){
		OOVR_FALSE_ABORT(pXrGetVulkanGraphicsDeviceKHR);
		return pXrGetVulkanGraphicsDeviceKHR(instance, systemId, vkInstance, vkPhysicalDevice);
	}
	XrResult xrGetVulkanGraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsVulkanKHR* graphicsRequirements){
		OOVR_FALSE_ABORT(pXrGetVulkanGraphicsRequirementsKHR);
		return pXrGetVulkanGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#ifdef SUPPORT_GL
	XrResult xrGetOpenGLGraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsOpenGLKHR* graphicsRequirements){
		OOVR_FALSE_ABORT(pXrGetOpenGLGraphicsRequirementsKHR);
		return pXrGetOpenGLGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#ifdef SUPPORT_GLES
	XrResult xrGetOpenGLESGraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsOpenGLESKHR* graphicsRequirements){
		OOVR_FALSE_ABORT(pXrGetOpenGLESGraphicsRequirementsKHR);
		return pXrGetOpenGLESGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif

	PFN_xrGetVisibilityMaskKHR xrGetVisibilityMaskKHR = nullptr;

private:
#ifdef SUPPORT_DX
	PFN_xrGetD3D11GraphicsRequirementsKHR pXrGetD3D11GraphicsRequirementsKHR = nullptr;
#endif
#ifdef SUPPORT_VK
	PFN_xrGetVulkanGraphicsRequirementsKHR pXrGetVulkanGraphicsRequirementsKHR = nullptr;
	PFN_xrGetVulkanInstanceExtensionsKHR pXrGetVulkanInstanceExtensionsKHR = nullptr;
	PFN_xrGetVulkanDeviceExtensionsKHR pXrGetVulkanDeviceExtensionsKHR = nullptr;
	PFN_xrGetVulkanGraphicsDeviceKHR pXrGetVulkanGraphicsDeviceKHR = nullptr;
#endif
#ifdef SUPPORT_GL
	PFN_xrGetOpenGLGraphicsRequirementsKHR pXrGetOpenGLGraphicsRequirementsKHR = nullptr;
#endif
#ifdef SUPPORT_GLES
	PFN_xrGetOpenGLESGraphicsRequirementsKHR pXrGetOpenGLESGraphicsRequirementsKHR = nullptr;
#endif

};

// Put this here rather tha xrutil.h so not all files have to include vector
extern std::vector<XrViewConfigurationView> xr_views_list;
