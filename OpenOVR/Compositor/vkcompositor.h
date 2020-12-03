#pragma once

#include "compositor.h"

class VkCompositor : public Compositor {
public:
	VkCompositor(const vr::Texture_t* initialTexture);

	virtual ~VkCompositor() override;

	// Override
	virtual void Invoke(const vr::Texture_t* texture) override;

	virtual void Invoke(ovrEyeType eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov& layer) override;

	virtual void InvokeCubemap(const vr::Texture_t* textures) override;

	unsigned int GetFlags() override;

private:
	bool CheckChainCompatible(const vr::VRVulkanTextureData_t& tex, const ovrTextureSwapChainDesc& chainDesc, vr::EColorSpace colourSpace);

	bool submitVerticallyFlipped = false;
	ovrTextureSwapChainDesc chainDesc;
	uint64_t /*VkCommandPool*/ commandPool = 0;

	uint32_t graphicsQueueFamilyId = 0;
};
