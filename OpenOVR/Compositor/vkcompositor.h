#pragma once

#include "compositor.h"

class VkCompositor : public Compositor {
public:
	VkCompositor(const vr::Texture_t* initialTexture, class TemporaryVk* target);

	~VkCompositor() override;

	// Override
	void Invoke(const vr::Texture_t* texture) override;

	void Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& viewport) override;

	void InvokeCubemap(const vr::Texture_t* textures) override;

	unsigned int GetFlags() override;

private:
	static bool CheckChainCompatible(const vr::VRVulkanTextureData_t& tex, const XrSwapchainCreateInfo& chainDesc, vr::EColorSpace colourSpace);

	void SetupMappedImages(VkCommandBuffer appCmdBuffer, std::vector<VkImageMemoryBarrier>& rtBarriers, std::vector<VkImageMemoryBarrier>& appBarriers);
	void MapMemoryToApp(VkDeviceMemory& rtMem, VkDeviceMemory& appMem, int memTypeIdx, VkDeviceSize size);
	VkDeviceSize BindNextMemoryToImage(VkDevice dev, VkImage img, VkDeviceMemory mem, VkDeviceSize offset);

	class TemporaryVk* target = nullptr;

	// These resources live in the runtime's VkDevice
	std::vector<XrSwapchainImageVulkanKHR> swapchainImages;
	VkImage rtImage = VK_NULL_HANDLE;
	VkCommandPool rtCommandPool = VK_NULL_HANDLE;
	VkDeviceMemory rtSharedMem = VK_NULL_HANDLE;

	// These resources live in the app's VkDevice
	VkDevice appDevice = VK_NULL_HANDLE;
	VkQueue appQueue = VK_NULL_HANDLE;
	VkCommandPool appCommandPool = VK_NULL_HANDLE;
	VkDeviceMemory appSharedMem = VK_NULL_HANDLE;
	VkImage appImage = VK_NULL_HANDLE;
};
