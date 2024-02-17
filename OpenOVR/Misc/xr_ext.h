//
// Created by ZNix on 25/10/2020.
//

#pragma once

// TODO Turtle1331 it would be nicer to include OpenXR-SDK's xr_dependencies.h if possible
#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
#include <d3d11.h>
#define XR_USE_GRAPHICS_API_D3D11
#endif

#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)
#include <d3d12.h>
#define XR_USE_GRAPHICS_API_D3D12
#endif

#ifdef SUPPORT_GL
#include <GL/gl.h>
#define XR_USE_GRAPHICS_API_OPENGL

#ifndef _WIN32
// Currently just support XLIB
#define XR_USE_PLATFORM_XLIB
#include <GL/glx.h>
// #define XR_USE_PLATFORM_XCB
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
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHICS_API_D3D11 = 0x0001;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHICS_API_D3D12 = 0x0002;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHICS_API_GL = 0x0004;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHICS_API_GLES = 0x0008;
static const XrGraphicsApiSupportedFlags XR_SUPPORTED_GRAPHICS_API_VK = 0x0010;

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
	XrExt(XrGraphicsApiSupportedFlags apis, const std::vector<const char*>& extensions);

	bool G2Controller_Available() { return supportsG2Controller; }
	bool xrGetVisibilityMaskKHR_Available() { return pfnXrGetVisibilityMaskKHR != nullptr; }
	XrResult xrGetVisibilityMaskKHR(
	    XrSession session,
	    XrViewConfigurationType viewConfigurationType,
	    uint32_t viewIndex,
	    XrVisibilityMaskTypeKHR visibilityMaskType,
	    XrVisibilityMaskKHR* visibilityMask)
	{
		OOVR_FALSE_ABORT(pfnXrGetVisibilityMaskKHR);
		return pfnXrGetVisibilityMaskKHR(session, viewConfigurationType, viewIndex, visibilityMaskType, visibilityMask);
	}

	bool handTrackingExtensionAvailable() { return pfnXrCreateHandTrackerExt != nullptr; }
	XrResult xrCreateHandTrackerEXT(XrSession session, const XrHandTrackerCreateInfoEXT* createInfo, XrHandTrackerEXT* handTracker)
	{
		OOVR_FALSE_ABORT(pfnXrCreateHandTrackerExt);
		return pfnXrCreateHandTrackerExt(session, createInfo, handTracker);
	}
	XrResult xrDestroyHandTrackerEXT(XrHandTrackerEXT handTracker)
	{
		OOVR_FALSE_ABORT(pfnXrDestroyHandTrackerExt);
		return pfnXrDestroyHandTrackerExt(handTracker);
	}
	XrResult xrLocateHandJointsEXT(XrHandTrackerEXT handTracker, const XrHandJointsLocateInfoEXT* locateInfo, XrHandJointLocationsEXT* locations)
	{
		OOVR_FALSE_ABORT(pfnXrLocateHandJointsExt);
		return pfnXrLocateHandJointsExt(handTracker, locateInfo, locations);
	}

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	bool xrGetD3D11GraphicsRequirementsKHR_Available()
	{
		return pfnXrGetD3D11GraphicsRequirementsKHR != nullptr;
	}
	XrResult xrGetD3D11GraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsD3D11KHR* graphicsRequirements)
	{
		OOVR_FALSE_ABORT(pfnXrGetD3D11GraphicsRequirementsKHR);
		return pfnXrGetD3D11GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)
	bool xrGetD3D12GraphicsRequirementsKHR_Available()
	{
		return pfnXrGetD3D12GraphicsRequirementsKHR != nullptr;
	}
	XrResult xrGetD3D12GraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsD3D12KHR* graphicsRequirements)
	{
		OOVR_FALSE_ABORT(pfnXrGetD3D12GraphicsRequirementsKHR);
		return pfnXrGetD3D12GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#ifdef SUPPORT_VK
	bool xrGetVulkanInstanceExtensionsKHR_Available()
	{
		return pfnXrGetVulkanInstanceExtensionsKHR != nullptr;
	}
	XrResult xrGetVulkanInstanceExtensionsKHR(XrInstance instance, XrSystemId systemId, uint32_t bufferCapacityInput, uint32_t* bufferCountOutput, char* buffer)
	{
		OOVR_FALSE_ABORT(pfnXrGetVulkanInstanceExtensionsKHR);
		return pfnXrGetVulkanInstanceExtensionsKHR(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer);
	}
	bool xrGetVulkanDeviceExtensionsKHR_Available() { return pfnXrGetVulkanDeviceExtensionsKHR != nullptr; }
	XrResult xrGetVulkanDeviceExtensionsKHR(XrInstance instance, XrSystemId systemId, uint32_t bufferCapacityInput, uint32_t* bufferCountOutput, char* buffer)
	{
		OOVR_FALSE_ABORT(pfnXrGetVulkanDeviceExtensionsKHR);
		return pfnXrGetVulkanDeviceExtensionsKHR(instance, systemId, bufferCapacityInput, bufferCountOutput, buffer);
	}
	bool xrGetVulkanGraphicsDeviceKHR_Available() { return pfnXrGetVulkanGraphicsDeviceKHR != nullptr; }
	XrResult xrGetVulkanGraphicsDeviceKHR(XrInstance instance, XrSystemId systemId, VkInstance vkInstance, VkPhysicalDevice* vkPhysicalDevice)
	{
		OOVR_FALSE_ABORT(pfnXrGetVulkanGraphicsDeviceKHR);
		return pfnXrGetVulkanGraphicsDeviceKHR(instance, systemId, vkInstance, vkPhysicalDevice);
	}
	bool xrGetVulkanGraphicsRequirementsKHR_Available() { return pfnXrGetVulkanGraphicsRequirementsKHR != nullptr; }
	XrResult xrGetVulkanGraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsVulkanKHR* graphicsRequirements)
	{
		OOVR_FALSE_ABORT(pfnXrGetVulkanGraphicsRequirementsKHR);
		return pfnXrGetVulkanGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#ifdef SUPPORT_GL
	bool xrGetOpenGLGraphicsRequirementsKHR_Available()
	{
		return pfnXrGetOpenGLGraphicsRequirementsKHR != nullptr;
	}
	XrResult xrGetOpenGLGraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsOpenGLKHR* graphicsRequirements)
	{
		OOVR_FALSE_ABORT(pfnXrGetOpenGLGraphicsRequirementsKHR);
		return pfnXrGetOpenGLGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif
#ifdef SUPPORT_GLES
	bool xrGetOpenGLESGraphicsRequirementsKHR_Available()
	{
		return pfnXrGetOpenGLESGraphicsRequirementsKHR != nullptr;
	}
	XrResult xrGetOpenGLESGraphicsRequirementsKHR(XrInstance instance, XrSystemId systemId, XrGraphicsRequirementsOpenGLESKHR* graphicsRequirements)
	{
		OOVR_FALSE_ABORT(pfnXrGetOpenGLESGraphicsRequirementsKHR);
		return pfnXrGetOpenGLESGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
	}
#endif

private:
	PFN_xrGetVisibilityMaskKHR pfnXrGetVisibilityMaskKHR = nullptr;
	PFN_xrCreateHandTrackerEXT pfnXrCreateHandTrackerExt = nullptr;
	PFN_xrDestroyHandTrackerEXT pfnXrDestroyHandTrackerExt = nullptr;
	PFN_xrLocateHandJointsEXT pfnXrLocateHandJointsExt = nullptr;
	bool supportsG2Controller = false;

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)
	PFN_xrGetD3D11GraphicsRequirementsKHR pfnXrGetD3D11GraphicsRequirementsKHR = nullptr;
#endif
#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)
	PFN_xrGetD3D12GraphicsRequirementsKHR pfnXrGetD3D12GraphicsRequirementsKHR = nullptr;
#endif
#ifdef SUPPORT_VK
	PFN_xrGetVulkanGraphicsRequirementsKHR pfnXrGetVulkanGraphicsRequirementsKHR = nullptr;
	PFN_xrGetVulkanInstanceExtensionsKHR pfnXrGetVulkanInstanceExtensionsKHR = nullptr;
	PFN_xrGetVulkanDeviceExtensionsKHR pfnXrGetVulkanDeviceExtensionsKHR = nullptr;
	PFN_xrGetVulkanGraphicsDeviceKHR pfnXrGetVulkanGraphicsDeviceKHR = nullptr;
#endif
#ifdef SUPPORT_GL
	PFN_xrGetOpenGLGraphicsRequirementsKHR pfnXrGetOpenGLGraphicsRequirementsKHR = nullptr;
#endif
#ifdef SUPPORT_GLES
	PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnXrGetOpenGLESGraphicsRequirementsKHR = nullptr;
#endif
};

// Put this here rather tha xrutil.h so not all files have to include vector
extern std::vector<XrViewConfigurationView> xr_views_list;
