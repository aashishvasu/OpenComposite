#pragma once

#include "compositor.h"

class GLBaseCompositor : public Compositor {
public:
	explicit GLBaseCompositor() = default;

	// Override
	void Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds) override;

	void Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& viewport) override;

	void InvokeCubemap(const vr::Texture_t* textures) override;

protected:
	/**
	 * Read the runtime-created swapchain names to [images] using the GL or GLES OpenXR structs.
	 */
	virtual void ReadSwapchainImages() = 0;

	void CheckCreateSwapChain(int width, int height, vr::EColorSpace c_space, GLsizei format);

	/**
	 * 'Normalise' an OpenGL internalFormat. glCopyImageSubData doesn't need exactly the same
	 * types, so if the game uses a format the runtime doesn't perfectly support we can cover
	 * up using this.
	 */
	static GLuint NormaliseFormat(vr::EColorSpace c_space, GLsizei rawFormat);

	GLuint fboId = 0;

	std::vector<GLuint> images;
};

#ifdef SUPPORT_GL
class GLCompositor : public GLBaseCompositor {
public:
	explicit GLCompositor(GLuint initialTexture);

protected:
	void ReadSwapchainImages() override;
};
#endif
