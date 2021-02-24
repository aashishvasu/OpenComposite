#include "stdafx.h"

#if defined(SUPPORT_VK)

// Required for the close(2) call for the texture shared memory on Linux
#ifndef _WIN32
#include <unistd.h>
#endif

#include "../../DrvOpenXR/tmp_gfx/TemporaryVk.h"
#include "vkcompositor.h"

#include <vulkan/vulkan.h>

using namespace std;

#define ERR(msg)                                                                                                                                         \
	do {                                                                                                                                                 \
		std::string str = "Hit Vulkan-related error " + string(msg) + " at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
		OOVR_ABORT(str.c_str());                                                                                                                         \
	} while (0)

// Start recording into a new command buffer that will only be executed once.
static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	OOVR_FAILED_VK_ABORT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	OOVR_FAILED_VK_ABORT(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

static void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	OOVR_FAILED_VK_ABORT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	OOVR_FAILED_VK_ABORT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

	OOVR_FAILED_VK_ABORT(vkQueueWaitIdle(queue));

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

/////////

VkCompositor::VkCompositor(const vr::Texture_t* initialTexture, class TemporaryVk* target)
    : target(target)
{
	auto* tex = (vr::VRVulkanTextureData_t*)initialTexture->handle;

	appDevice = tex->m_pDevice;
	appQueue = tex->m_pQueue;

	VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	OOVR_FAILED_VK_ABORT(vkCreateCommandPool(tex->m_pDevice, &poolInfo, nullptr, &appCommandPool));
	OOVR_FAILED_VK_ABORT(vkCreateCommandPool(target->device, &poolInfo, nullptr, &rtCommandPool));
}

VkCompositor::~VkCompositor()
{
	vkDestroyCommandPool(appDevice, appCommandPool, nullptr);
	vkDestroyCommandPool(target->device, rtCommandPool, nullptr);

	if (appImage != VK_NULL_HANDLE)
		vkDestroyImage(appDevice, appImage, nullptr);

	if (rtImage != VK_NULL_HANDLE)
		vkDestroyImage(target->device, rtImage, nullptr);

	if (appSharedMem != VK_NULL_HANDLE)
		vkFreeMemory(appDevice, appSharedMem, nullptr);

	if (rtSharedMem != VK_NULL_HANDLE)
		vkFreeMemory(target->device, rtSharedMem, nullptr);
}

void VkCompositor::Invoke(const vr::Texture_t* texture)
{
	const vr::VRVulkanTextureData_t* tex = (vr::VRVulkanTextureData_t*)texture->handle;

	if (!tex) {
		ERR("Cannot use NULL Vulkan image data (VRVulkanTextureData_t)");
	}

	OOVR_FALSE_ABORT(appDevice == tex->m_pDevice);

	//

	// Set up a command buffer for both the app and runtime devices
	VkCommandBuffer appCommandBuffer = beginSingleTimeCommands(tex->m_pDevice, appCommandPool);
	VkCommandBuffer rtCommandBuffer = beginSingleTimeCommands(target->device, rtCommandPool);

	bool usable = chain != XR_NULL_HANDLE && CheckChainCompatible(*tex, createInfo, texture->eColorSpace);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		// First, delete the old chain if necessary
		if (chain)
			xrDestroySwapchain(chain);

		// Make eye render buffer
		createInfo = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		createInfo.createFlags = 0;
		createInfo.usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
		createInfo.faceCount = 1;
		createInfo.width = tex->m_nWidth;
		createInfo.height = tex->m_nHeight;
		createInfo.mipCount = 1;
		createInfo.sampleCount = tex->m_nSampleCount;
		createInfo.arraySize = 1;

		// FIXME do the same colourspace conversion hoop-jumping as the D3D11 compositor
		createInfo.format = tex->m_nFormat;

		OOVR_FAILED_XR_ABORT(xrCreateSwapchain(xr_session, &createInfo, &chain));

		// Perform a layout transition, since the textures come
		//  as VK_IMAGE_LAYOUT_UNDEFINED, at least at the time of writing.

		uint32_t chainLength = 0;
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, 0, &chainLength, nullptr));
		swapchainImages.resize(chainLength);
		for (XrSwapchainImageVulkanKHR& swapchainImage : swapchainImages)
			swapchainImage.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, swapchainImages.size(), &chainLength, (XrSwapchainImageBaseHeader*)swapchainImages.data()));

		vector<VkImageMemoryBarrier> rtBarriers, appBarriers;
		for (const XrSwapchainImageVulkanKHR& swapchainImage : swapchainImages) {
			VkImage image = swapchainImage.image;

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

			rtBarriers.push_back(barrier);
		}

		SetupMappedImages(rtCommandBuffer, rtBarriers, appBarriers);

		vkCmdPipelineBarrier(
		    rtCommandBuffer,
		    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Being inefficient here is fine, only runs on setup
		    0,
		    0, nullptr,
		    0, nullptr,
		    rtBarriers.size(), rtBarriers.data());

		vkCmdPipelineBarrier(
		    appCommandBuffer,
		    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Being inefficient here is fine, only runs on setup
		    0,
		    0, nullptr,
		    0, nullptr,
		    appBarriers.size(), appBarriers.data());
	}

	// First find the relevant image to render to
	XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t currentIndex;
	OOVR_FAILED_XR_ABORT(xrAcquireSwapchainImage(chain, &acquireInfo, &currentIndex));

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

	vkCmdCopyImage(appCommandBuffer,
	    // Valve defines a single correct image layout, which certainly
	    //  makes things easier here.
	    (VkImage)tex->m_nImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,

	    // Set in SetupMappedImages
	    appImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

	    // Infromation about which subresource to copy, and which area to copy
	    1, &region);

	// Repeat the image copy, but on the runtime side
	vkCmdCopyImage(rtCommandBuffer,
	    // Set in SetupMappedImages
	    rtImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,

	    // Set during the transition
	    swapchainImages.at(currentIndex).image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

	    // Information about which subresource to copy, and which area to copy
	    1, &region);

	// Copy the images from the application to the shared memory pool - this is fenced on the application finishing rendering
	endSingleTimeCommands(tex->m_pDevice, appCommandPool, appCommandBuffer, tex->m_pQueue);

	// Wait until the swapchain is ready - this makes sure the compositor isn't writing to it
	// We don't have to pass in currentIndex since it uses the oldest acquired-but-not-waited-on
	// image, so we should be careful with concurrency here.
	XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	OOVR_FAILED_XR_ABORT(xrWaitSwapchainImage(chain, &waitInfo));

	// Submit the write commands, which actually copy across the image
	endSingleTimeCommands(target->device, rtCommandPool, rtCommandBuffer, target->queue);

	// Release the swapchain - OpenXR will use the last-released image in a swapchain
	XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	OOVR_FAILED_XR_ABORT(xrReleaseSwapchainImage(chain, &releaseInfo));
}

void VkCompositor::Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* ptrBounds,
    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& layer)
{

	// Copy the texture across
	Invoke(texture);

	vr::VRVulkanTextureData_t& tex = *(vr::VRVulkanTextureData_t*)texture->handle;

	// Set the viewport up
	XrSwapchainSubImage& subImage = layer.subImage;
	subImage.swapchain = chain;
	subImage.imageArrayIndex = 0; // This is *not* the swapchain index
	XrRect2Di& viewport = subImage.imageRect;

	// TODO deduplicate with dx11compositor, and use for all compositors
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
			// TODO support vertically flipped images
			XR_STUBBED(); // submitVerticallyFlipped = true;
			float newMax = bounds.vMin;
			bounds.vMin = bounds.vMax;
			bounds.vMax = newMax;
		} else {
			// submitVerticallyFlipped = false;
		}

		viewport.offset.x = (int)(bounds.uMin * tex.m_nWidth);
		viewport.offset.y = (int)(bounds.vMin * tex.m_nHeight);
		viewport.extent.width = (int)((bounds.uMax - bounds.uMin) * tex.m_nWidth);
		viewport.extent.height = (int)((bounds.vMax - bounds.vMin) * tex.m_nHeight);
	} else {
		viewport.offset.x = viewport.offset.y = 0;
		viewport.extent.width = tex.m_nWidth;
		viewport.extent.height = tex.m_nHeight;

		// submitVerticallyFlipped = false;
	}
}

void VkCompositor::InvokeCubemap(const vr::Texture_t* textures)
{
	OOVR_ABORT("VkCompositor::InvokeCubemap: Not yet supported!");
}

unsigned int VkCompositor::GetFlags()
{
	// Since we don't have layer flags anymore, we can probably get rid of this function in all compositors
	XR_STUBBED();
}

bool VkCompositor::CheckChainCompatible(const vr::VRVulkanTextureData_t& tex, const XrSwapchainCreateInfo& chainDesc, vr::EColorSpace colourSpace)
{
	bool usable = true;
#define FAIL(name)                             \
	do {                                       \
		usable = false;                        \
		OOVR_LOG("Resource mismatch: " #name); \
	} while (0)
#define CHECK(name, chainName)           \
	if (tex.name != chainDesc.chainName) \
	FAIL(name)

	CHECK(m_nWidth, width);
	CHECK(m_nHeight, height);
	CHECK(m_nSampleCount, sampleCount);

	// FIXME colourspace conversion
	(void)colourSpace;
	CHECK(m_nFormat, format);
	// if (chainDesc.Format != vkToOvrFormat((VkFormat)tex.m_nFormat, colourSpace)) FAIL(Format);

#undef CHECK
#undef FAIL

	return usable;
}

void VkCompositor::SetupMappedImages(VkCommandBuffer appCmdBuffer, std::vector<VkImageMemoryBarrier>& rtBarriers, std::vector<VkImageMemoryBarrier>& appBarriers)
{
	// Allocate memory shared between both devices, and within that build an image on both devices - so copying to one will
	// make the result show up in the other. This is how we transfer images between the app and runtime.

	//////////////////////////////////
	/// Texture allocation stuff /////
	//////////////////////////////////

	// Since these textures tend to be huge (like, 45MiB for one eye in the Rift S at sampleCount=4) only make one of them, rather than
	// one per image in the swapchain.
	VkImageCreateInfo imgInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.format = (VkFormat)createInfo.format;
	imgInfo.extent = VkExtent3D{ createInfo.width, createInfo.height, 1 };
	imgInfo.mipLevels = createInfo.mipCount;
	imgInfo.arrayLayers = createInfo.arraySize;
	imgInfo.samples = (VkSampleCountFlagBits)createInfo.sampleCount;
	imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imgInfo.queueFamilyIndexCount = 0; // This must be set if we're in the concurrent sharing mode
	imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// TODO createInfo.faceCount - what's that for?

	// Create the image on both the runtime and app sides
	OOVR_FAILED_VK_ABORT(vkCreateImage(target->device, &imgInfo, nullptr, &rtImage));
	OOVR_FAILED_VK_ABORT(vkCreateImage(appDevice, &imgInfo, nullptr, &appImage));

	// Find how large the memory needs to be
	VkDeviceSize imgSize = BindNextMemoryToImage(target->device, rtImage, VK_NULL_HANDLE, 0);

	// Add a barrier to put the the images in the same format
	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // SRC_OPTIMAL would make just as much sense here

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = appImage;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = 0; // TODO
	barrier.dstAccessMask = 0; // TODO

	appBarriers.push_back(barrier);

	barrier.image = rtImage;
	rtBarriers.push_back(barrier);

	//////////////////////////////////
	/// Memory allocation stuff //////
	//////////////////////////////////

	// Get the memory properties for our physical device - since both the app and runtime are
	// on the same physical device, we only need to look this up once.
	// Also, find the best heap for this memory. It just needs to be device-local and that's it.
	// (Is there a way to find what heap a VkImage is allocated on? Maybe that would be good for
	//  performance to always put this buffer on the same heap as the app's images?)
	VkPhysicalDeviceMemoryProperties memProps{};
	vkGetPhysicalDeviceMemoryProperties(target->physicalDevice, &memProps);

	int bestHeapId = -1;
	for (int i = 0; i < memProps.memoryHeapCount; i++) {
		if (memProps.memoryHeaps[i].flags | VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			bestHeapId = i;
			break;
		}
	}
	OOVR_FALSE_ABORT(bestHeapId != -1); // This is forbidden by the Vulkan standard - there must always be a device-local heap.

	int bestTypeId = -1;
	for (int i = 0; i < memProps.memoryTypeCount; i++) {
		if (memProps.memoryTypes[i].heapIndex == bestHeapId) {
			bestTypeId = i;
			break;
		}
	}
	OOVR_FALSE_ABORT(bestTypeId != -1);

	// Allocate a pool of memory that's available on both devices
	MapMemoryToApp(rtSharedMem, appSharedMem, bestTypeId, imgSize);

	// Bind that memory to the images
	BindNextMemoryToImage(target->device, rtImage, rtSharedMem, 0);
	BindNextMemoryToImage(appDevice, appImage, appSharedMem, 0);
}

#ifdef _WIN32
void VkCompositor::MapMemoryToApp(VkDeviceMemory& rtMem, VkDeviceMemory& appMem, int memTypeIdx, VkDeviceSize size)
{
	// Allocate the host-side memory
	VkExportMemoryAllocateInfo exportInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO };
	exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	VkMemoryAllocateInfo hostAlloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, &exportInfo };
	hostAlloc.memoryTypeIndex = memTypeIdx;
	hostAlloc.allocationSize = size;

	OOVR_FAILED_VK_ABORT(vkAllocateMemory(target->device, &hostAlloc, nullptr, &rtMem));

	// Export a handle representing the memory on the runtime side
	auto vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(target->device, "vkGetMemoryWin32HandleKHR");
	OOVR_FALSE_ABORT(vkGetMemoryWin32HandleKHR != nullptr);

	VkMemoryGetWin32HandleInfoKHR getHandle = { VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR };
	getHandle.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
	getHandle.memory = rtMem;
	HANDLE handle;
	OOVR_FAILED_VK_ABORT(vkGetMemoryWin32HandleKHR(target->device, &getHandle, &handle));
	OOVR_FALSE_ABORT(handle != nullptr);

	// Import the memory on the app side
	VkImportMemoryWin32HandleInfoKHR importMem = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR };
	importMem.handleType = getHandle.handleType;
	importMem.handle = handle;

	VkMemoryAllocateInfo importAllocate = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, &importMem };
	importAllocate.memoryTypeIndex = memTypeIdx;
	importAllocate.allocationSize = size;

	OOVR_FAILED_VK_ABORT(vkAllocateMemory(appDevice, &importAllocate, nullptr, &appMem));

	// There seems to be conflicting information on whether we should close this or not - the documentation says that
	// importing a NT handle (which AFAIK this is, according to the HANDLE_TYPE_OPAQUE_WIN32_BIT) but the issues for
	// the extension says otherwise.
	// CloseHandle(handle); // Just ignore the error if present, can't do anything about it
}

#else
void VkCompositor::MapMemoryToApp(VkDeviceMemory& rtMem, VkDeviceMemory& appMem, int memTypeIdx, VkDeviceSize size)
{
	// Allocate the host-side memory
	VkExportMemoryAllocateInfo exportInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO };
	exportInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

	VkMemoryAllocateInfo hostAlloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, &exportInfo };
	hostAlloc.memoryTypeIndex = memTypeIdx;
	hostAlloc.allocationSize = size;

	OOVR_FAILED_VK_ABORT(vkAllocateMemory(target->device, &hostAlloc, nullptr, &rtMem));

	// Export a handle representing the memory on the runtime side
	auto vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(target->device, "vkGetMemoryFdKHR");
	OOVR_FALSE_ABORT(vkGetMemoryFdKHR != nullptr);

	VkMemoryGetFdInfoKHR getHandle = { VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR };
	getHandle.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
	getHandle.memory = rtMem;
	int fd = -1;
	OOVR_FAILED_VK_ABORT(vkGetMemoryFdKHR(target->device, &getHandle, &fd));
	OOVR_FALSE_ABORT(fd != -1);

	// Import the memory on the app side
	VkImportMemoryFdInfoKHR importMem = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR };
	importMem.handleType = getHandle.handleType;
	importMem.fd = fd;

	VkMemoryAllocateInfo importAllocate = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, &importMem };
	importAllocate.memoryTypeIndex = memTypeIdx;
	importAllocate.allocationSize = size;

	OOVR_FAILED_VK_ABORT(vkAllocateMemory(appDevice, &importAllocate, nullptr, &appMem));

	// There seems to be conflicting information on whether we should close this or not - the documentation says that
	close(fd); // Just ignore the error if present, can't do anything about it
}
#endif

VkDeviceSize VkCompositor::BindNextMemoryToImage(VkDevice dev, VkImage img, VkDeviceMemory mem, VkDeviceSize offset)
{
	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(dev, img, &memReq);

	VkDeviceSize overhang = offset % memReq.alignment;
	VkDeviceSize padding = overhang == 0 ? 0 : memReq.alignment - overhang;

	if (mem != VK_NULL_HANDLE)
		OOVR_FAILED_VK_ABORT(vkBindImageMemory(dev, img, mem, offset + padding));

	return padding + memReq.size;
}

#endif
