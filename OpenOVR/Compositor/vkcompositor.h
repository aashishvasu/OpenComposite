#pragma once

#include "compositor.h"

// BAD NO NO NO NO. NO GLOBAL STATE
// The actual purpose of these, from Moses, is so we can assert if the client tries to switch tehse up on us.
extern VkPhysicalDevice expectedPhysicalDevice;
extern VkDevice expectedDevice;
extern VkQueue expectedQueue;

class VkCompositor : public Compositor {
public:
	VkCompositor(const vr::Texture_t* initialTexture);

	~VkCompositor() override;

	// Override
	void Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds) override;

	void Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& viewport) override;

	void InvokeCubemap(const vr::Texture_t* textures) override;

private:
	static bool CheckChainCompatible(const vr::VRVulkanTextureData_t& tex, const XrSwapchainCreateInfo& chainDesc, vr::EColorSpace colourSpace);

	void SetupMappedImages(VkCommandBuffer appCmdBuffer, std::vector<VkImageMemoryBarrier>& rtBarriers, std::vector<VkImageMemoryBarrier>& appBarriers);
	void MapMemoryToApp(VkDeviceMemory& rtMem, VkDeviceMemory& appMem, int memTypeIdx, VkDeviceSize size);
	VkDeviceSize BindNextMemoryToImage(VkDevice dev, VkImage img, VkDeviceMemory mem, VkDeviceSize offset);

	// These resources live in the runtime's VkDevice
	std::vector<XrSwapchainImageVulkanKHR> swapchainImages;

	// These resources live in the app's VkDevice
	VkDevice appDevice = VK_NULL_HANDLE;
	VkQueue appQueue = VK_NULL_HANDLE;
	VkCommandPool appCommandPool = VK_NULL_HANDLE;
	VkDeviceMemory appSharedMem = VK_NULL_HANDLE;
	VkImage appImage = VK_NULL_HANDLE;
};
