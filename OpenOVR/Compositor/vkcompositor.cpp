#include "stdafx.h"
#include "compositor.h"

#if defined(SUPPORT_VK)

#include "libovr_wrapper.h"
#include "OVR_CAPI_Vk.h"
#include <vulkan/vulkan.h>

using namespace std;
#define OVSS (*ovr::session)

#define ERR(msg) { \
	std::string str = "Hit Vulkan-related error " + string(msg) + " at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_LOG(str.c_str()); \
	MessageBoxA(NULL, str.c_str(), "Errored func!", MB_OK); \
	/**((int*)NULL) = 0;*/\
	throw str; \
}

ovrTextureFormat vkToOvrFormat(VkFormat vk, vr::EColorSpace colourSpace) {
	// TODO is this really how it should work?
	// No idea why or how or what, but for now just force SRGB on as otherwise
	// it causes trouble.
	bool useSrgb = true; // colourSpace != vr::ColorSpace_Auto;

	switch (vk) {

	case VK_FORMAT_R8G8B8A8_SRGB:
		return OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	case VK_FORMAT_B8G8R8A8_SRGB:
		return OVR_FORMAT_B8G8R8A8_UNORM_SRGB;

		// TODO The following formats are supported by SteamVR, and not supported by OpenComposite:
		// VK_FORMAT_R8G8B8A8_UNORM
		// VK_FORMAT_B8G8R8A8_UNORM
		// VK_FORMAT_R32G32B32A32_SFLOAT
		// VK_FORMAT_R32G32B32_SFLOAT
		// VK_FORMAT_R16G16B16A16_SFLOAT
		// VK_FORMAT_A2R10G10B10_UINT_PACK32

	default:
		OOVR_LOGF("Unknown Vulkan format: %d", vk);
		ERR("Unknown Vulkan format in image!");
	}

	return OVR_FORMAT_UNKNOWN;
}

static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

static void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, uint32_t graphicsQueueFamilyId) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkQueue graphicsQueue;
	vkGetDeviceQueue(device, graphicsQueueFamilyId, 0, &graphicsQueue);
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

/////////

VkCompositor::VkCompositor(const vr::Texture_t *initialTexture) {
	vr::VRVulkanTextureData_t *tex = (vr::VRVulkanTextureData_t*) initialTexture->handle;

	// Tell Oculus which queue to sync on
	ovr_SetSynchronizationQueueVk(OVSS, tex->m_pQueue);

	// Find the queue family ID for the graphics queue family:

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(tex->m_pPhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(tex->m_pPhysicalDevice, &queueFamilyCount, queueFamilies.data());

	for (unsigned int i = 0; i < queueFamilies.size(); i++) {
		const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueFamilyId = i;
			goto found;
		}
	}
	ERR("Vulkan: no available transfer queue!");

found:

	// Create a command pool for it
	// TODO only create one for all instances of VkCompositor

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsQueueFamilyId;
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(tex->m_pDevice, &poolInfo, nullptr, (VkCommandPool*)&commandPool) != VK_SUCCESS) {
		ERR("failed to create command pool!");
	}
}

VkCompositor::~VkCompositor() {
	// TODO free the command pool:
	// vkDestroyCommandPool(device, (VkCommandPool) commandPool, nullptr);
}

void VkCompositor::Invoke(const vr::Texture_t * texture) {
	const vr::VRVulkanTextureData_t *tex = (vr::VRVulkanTextureData_t*) texture->handle;

	if (!tex) {
		ERR("Cannot use NULL Vulkan image data (VRVulkanTextureData_t)");
	}

	//

	ovrTextureSwapChainDesc &desc = chainDesc;

	bool usable = chain == NULL ? false : CheckChainCompatible(*tex, desc, texture->eColorSpace);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		// First, delete the old chain if necessary
		if (chain)
			ovr_DestroyTextureSwapChain(OVSS, chain);

		// Make eye render buffer
		desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = tex->m_nWidth;
		desc.Height = tex->m_nHeight;
		desc.Format = vkToOvrFormat((VkFormat)tex->m_nFormat, texture->eColorSpace);
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleCount = tex->m_nSampleCount;
		desc.StaticImage = ovrFalse;

		desc.MiscFlags = ovrTextureMisc_None;
		desc.BindFlags = ovrTextureBind_None;

		ovrResult result = ovr_CreateTextureSwapChainVk(OVSS, tex->m_pDevice, &desc, &chain);
		if (!OVR_SUCCESS(result))
			ERR("Cannot create DX texture swap chain " + to_string(result));

		// Perform a layout transition, since the textures come
		//  as VK_IMAGE_LAYOUT_UNDEFINED, at least at the time of writing.

		VkCommandBuffer commandBuffer = beginSingleTimeCommands(tex->m_pDevice, (VkCommandPool)commandPool);

		int chainLength = 0;
		ovr_GetTextureSwapChainLength(OVSS, chain, &chainLength);

		vector<VkImageMemoryBarrier> barriers;

		for (int i = 0; i < chainLength; ++i) {
			VkImage image = NULL;
			ovr_GetTextureSwapChainBufferVk(OVSS, chain, i, &image);

			// for each texture
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.srcAccessMask = 0; // TODO
			barrier.dstAccessMask = 0; // TODO

			barriers.push_back(barrier);
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			barriers.size(), barriers.data()
		);

		endSingleTimeCommands(tex->m_pDevice, (VkCommandPool)commandPool, commandBuffer, graphicsQueueFamilyId);

	}

	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(OVSS, chain, &currentIndex);

	VkImage dst = NULL;
	ovr_GetTextureSwapChainBufferVk(OVSS, chain, currentIndex, &dst);

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(tex->m_pDevice, (VkCommandPool) commandPool);

	VkImageCopy region = {};
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.mipLevel = 0;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = 1;
	region.srcOffset = { 0, 0, 0 };
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.mipLevel = 0;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = 1;
	region.dstOffset = { 0, 0, 0 };
	region.extent = { tex->m_nWidth, tex->m_nHeight, 1 };

	vkCmdCopyImage(commandBuffer,
		// Valve defines a single correct image layout, which certainly
		//  makes things easier here.
		(VkImage) tex->m_nImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,

		// Set during the transition
		dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

		// Infromation about which subresource to copy, and which area to copy
		1, &region);

	endSingleTimeCommands(tex->m_pDevice, (VkCommandPool)commandPool, commandBuffer, graphicsQueueFamilyId);
}

void VkCompositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * ptrBounds,
	vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	// Copy the texture across
	Invoke(texture);

	// Set the viewport up
	vr::VRVulkanTextureData_t &tex = *(vr::VRVulkanTextureData_t*) texture->handle;
	ovrRecti &viewport = layer.Viewport[eye];

	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
			submitVerticallyFlipped = true;
			float newMax = bounds.vMin;
			bounds.vMin = bounds.vMax;
			bounds.vMax = newMax;
		}
		else {
			submitVerticallyFlipped = false;
		}

		viewport.Pos.x = (int)(bounds.uMin * tex.m_nWidth);
		viewport.Pos.y = (int)(bounds.vMin * tex.m_nHeight);
		viewport.Size.w = (int)((bounds.uMax - bounds.uMin) * tex.m_nWidth);
		viewport.Size.h = (int)((bounds.vMax - bounds.vMin) * tex.m_nHeight);
	}
	else {
		viewport.Pos.x = viewport.Pos.y = 0;
		viewport.Size.w = tex.m_nWidth;
		viewport.Size.h = tex.m_nHeight;

		submitVerticallyFlipped = false;
	}
}

void VkCompositor::InvokeCubemap(const vr::Texture_t * textures) {
	OOVR_ABORT("VkCompositor::InvokeCubemap: Not yet supported!");
}

bool VkCompositor::CheckChainCompatible(const vr::VRVulkanTextureData_t &tex, const ovrTextureSwapChainDesc &chainDesc, vr::EColorSpace colourSpace) {
	bool usable = true;
#define FAIL(name) { \
	usable = false; \
	OOVR_LOG("Resource mismatch: " #name); \
}
#define CHECK(name, chainName) \
if(tex.name != chainDesc.chainName) FAIL(name);

	CHECK(m_nWidth, Width);
	CHECK(m_nHeight, Height);
	CHECK(m_nSampleCount, SampleCount);

	if (chainDesc.Format != vkToOvrFormat((VkFormat)tex.m_nFormat, colourSpace)) FAIL(Format);
#undef CHECK
#undef FAIL

	return usable;
}

#endif
