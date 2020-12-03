#pragma once

#include "compositor.h"

class GLCompositor : public Compositor {
public:
	GLCompositor(OVR::Sizei size);

	unsigned int GetFlags() override;

	// Override
	virtual void Invoke(const vr::Texture_t* texture) override;

	virtual void Invoke(ovrEyeType eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov& layer) override;

	virtual void InvokeCubemap(const vr::Texture_t* textures) override;

private:
	GLuint fboId = 0;
};
