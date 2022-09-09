#include "stdafx.h"

#if defined(SUPPORT_GL) || defined(SUPPORT_GLES)
#include "glcompositor.h"
#endif

// Import GL or GLES
#ifdef SUPPORT_GL
#include <GL/gl.h>
#endif
#ifdef SUPPORT_GLES
#include <GLES3/gl32.h>
#endif

// OpenGL compositor
#ifdef SUPPORT_GL

#include <algorithm>
#include <string>

// On Linux these seem to already be defined
#ifdef _WIN32
typedef void(APIENTRY* PFNGLGETTEXTURELEVELPARAMETERIVPROC)(GLuint texture, GLint level, GLenum pname, GLint* params);
typedef void(APIENTRY* PFNGLCOPYIMAGESUBDATAPROC)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
    GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
#endif

static PFNGLGETTEXTURELEVELPARAMETERIVPROC glGetTextureLevelParameteriv = nullptr;
static PFNGLCOPYIMAGESUBDATAPROC glCopyImageSubData = nullptr;

static void* getGlProcAddr(const char* name)
{
#ifdef _WIN32
	return (void*)wglGetProcAddress(name);
#else
	return (void*)glXGetProcAddress((const GLubyte*)name);
#endif
}

GLCompositor::GLCompositor(GLuint initialTexture)
{
	if (!glGetTextureLevelParameteriv) {
		glGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIVPROC)getGlProcAddr("glGetTextureLevelParameteriv");
		glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC)getGlProcAddr("glCopyImageSubData");
		if (!glGetTextureLevelParameteriv)
			OOVR_ABORT("Could not get function glGetTextureLevelParameteriv");

		if (!glCopyImageSubData)
			OOVR_ABORT("Could not get function glCopyImageSubData");
	}
}

void GLCompositor::ReadSwapchainImages()
{
	// Enumerate all the swapchain images
	uint32_t imageCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, 0, &imageCount, nullptr));
	auto handles = std::vector<XrSwapchainImageOpenGLKHR>(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
	OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)handles.data()));

	images.clear();
	for (const XrSwapchainImageOpenGLKHR& img : handles) {
		images.push_back(img.image);
	}
}

#endif

// BaseGLCompositor

#if defined(SUPPORT_GL) || defined(SUPPORT_GLES)

void GLBaseCompositor::Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds)
{
	// Clear any pre-existing OpenGL errors
	while (glGetError() != GL_NO_ERROR) {
	}

	// Double-cast to suppress CLion warning
	auto src = (GLuint)(intptr_t)texture->handle;

	CheckCreateSwapChain(src);

	// First reserve an image from the swapchain
	XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t currentIndex = 0;
	OOVR_FAILED_XR_ABORT(xrAcquireSwapchainImage(chain, &acquireInfo, &currentIndex));

	// Wait until the swapchain is ready - this makes sure the compositor isn't writing to it
	// We don't have to pass in currentIndex since it uses the oldest acquired-but-not-waited-on
	// image, so we should be careful with concurrency here.
	XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };

	// If the compositor is being slow, keep trying until we get through. We're not allowed to just
	// fail since the image has been acquired.
	// TODO make this stuff common across compositors, so this logic applies to all of them.
	XrResult res;
	do {
		OOVR_FAILED_XR_ABORT(res = xrWaitSwapchainImage(chain, &waitInfo));
	} while (res == XR_TIMEOUT_EXPIRED);

	// Actually copy the image across
	GLuint dst = images.at(currentIndex);
	glCopyImageSubData(
	    src, GL_TEXTURE_2D, 0, 0, 0, 0, // 0 == no mipmapping, next three are xyz
	    dst, GL_TEXTURE_2D, 0, 0, 0, 0, // Same as above but for the destination
	    createInfo.width, createInfo.height, 1 // Region of the output texture to copy into (in this case, everything)
	);

	// Abort if there was an OpenGL error
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		// Just warn, so we can find it in the logs - I fear it may crash in unexpected corner-cases otherwise.
		// OOVR_ABORTF("OpenGL texture copy failed with err %d", err);
		OOVR_LOG_ONCE("WARNING: OpenGL texture copy failed!");
	}

	// Release the swapchain - OpenXR will use the last-released image in a swapchain
	XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	OOVR_FAILED_XR_ABORT(xrReleaseSwapchainImage(chain, &releaseInfo));
}

void GLBaseCompositor::Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* ptrBounds,
    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& layer)
{
	// Copy the texture over
	Invoke(texture, ptrBounds);

	// TODO the image is vertically flipped; fix it

	// Set the viewport up
	// TODO deduplicate with dx11compositor, and use for all compositors
	XrSwapchainSubImage& subImage = layer.subImage;
	subImage.swapchain = chain;
	subImage.imageArrayIndex = 0; // This is *not* the swapchain index
	XrRect2Di& viewport = subImage.imageRect;
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
			float newMax = bounds.vMin;
			bounds.vMin = bounds.vMax;
			bounds.vMax = newMax;

			// Vertical flip not yet implemented!
			XR_STUBBED(); // submitVerticallyFlipped = true;
		} else {
			// submitVerticallyFlipped = false;
		}

		viewport.offset.x = (int)(bounds.uMin * createInfo.width);
		viewport.offset.y = (int)(bounds.vMin * createInfo.height);
		viewport.extent.width = (int)((bounds.uMax - bounds.uMin) * createInfo.width);
		viewport.extent.height = (int)((bounds.vMax - bounds.vMin) * createInfo.height);
	} else {
		viewport.offset.x = viewport.offset.y = 0;
		viewport.extent.width = createInfo.width;
		viewport.extent.height = createInfo.height;

		// submitVerticallyFlipped = false;
	}
}

void GLBaseCompositor::InvokeCubemap(const vr::Texture_t* textures)
{
	OOVR_ABORT("GLCompositor::InvokeCubemap: Not yet supported!");
}

void GLBaseCompositor::CheckCreateSwapChain(GLuint image)
{
	GLsizei width, height, rawFormat;
	glBindTexture(GL_TEXTURE_2D, image); // Sadly even GLES3.2 doesn't have glGetTextureLevelParameteriv which takes the image directly
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &rawFormat);

	// See the comment for NormaliseFormat as to why we're doing this
	GLuint format = NormaliseFormat(rawFormat);

	// Build out the info describing the swapchain we need
	XrSwapchainCreateInfo desc = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
	desc.faceCount = 1;
	desc.width = width;
	desc.height = height;
	desc.format = format;
	desc.mipCount = 1; // TODO srcDesc.MipLevels;
	desc.sampleCount = 1;
	desc.arraySize = 1;

	// If the format has changed (or this is the first call), continue on to create the swapchain
	if (memcmp(&desc, &createInfo, sizeof(desc)) == 0) {
		return;
	}

	// Make sure our format is supported
	uint32_t formatCount;
	OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainFormats(xr_session, 0, &formatCount, nullptr));
	std::vector<int64_t> formats(formatCount);
	OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainFormats(xr_session, formatCount, &formatCount, formats.data()));

	if (std::count(formats.begin(), formats.end(), format) == 0) {
		OOVR_LOG("Missing format for swapchain creation. Valid formats:");
		for (int64_t f : formats) {
			OOVR_LOGF("Valid format: %d", f);
		}
		OOVR_ABORTF("The runtime does not support the OpenGL format %d", format);
	}

	// Delete the old swapchain, if applicable
	if (chain) {
		OOVR_FAILED_XR_ABORT(xrDestroySwapchain(chain));
		chain = XR_NULL_HANDLE;
	}

	// Create the new one
	createInfo = desc;
	OOVR_FAILED_XR_ABORT(xrCreateSwapchain(xr_session, &desc, &chain));

	// Enumerate all the swapchain images
	ReadSwapchainImages();
}

GLuint GLBaseCompositor::NormaliseFormat(GLuint format)
{
	switch (format) {
	case GL_RGBA:
		return GL_RGBA8;
	default:
		return format;
	}
}

#endif
