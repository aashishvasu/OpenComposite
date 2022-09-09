#include "stdafx.h"

#ifdef SUPPORT_GLES

#include "glescompositor.h"

#include <GLES3/gl32.h>

#include <algorithm>
#include <string>

GLESCompositor::GLESCompositor() = default;

void GLESCompositor::ReadSwapchainImages()
{
	// Enumerate all the swapchain images
	uint32_t imageCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, 0, &imageCount, nullptr));
	auto handles = std::vector<XrSwapchainImageOpenGLESKHR>(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR });
	OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)handles.data()));

	images.clear();
	for (const XrSwapchainImageOpenGLESKHR& img : handles) {
		images.push_back(img.image);
	}
}

#endif
