#pragma once

#include "compositor.h"

class VkCompositor : public Compositor {
public:
	VkCompositor(const vr::Texture_t* initialTexture);

	~VkCompositor() override;

	void CopyToSwapchain(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, std::optional<XruEye> eye, vr::EVRSubmitFlags submitFlags) override;

	void InvokeCubemap(const vr::Texture_t* textures) override;

private:
	static bool CheckChainCompatible(const vr::VRVulkanTextureData_t& tex, const XrSwapchainCreateInfo& chainDesc, vr::EColorSpace colourSpace);

	// These resources live in the runtime's VkDevice
	std::vector<XrSwapchainImageVulkanKHR> swapchainImages;

	// These resources live in the app's VkDevice
	VkDevice appDevice = VK_NULL_HANDLE;
	VkQueue appQueue = VK_NULL_HANDLE;
	VkCommandPool appCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> appCommandBuffers{};
};
