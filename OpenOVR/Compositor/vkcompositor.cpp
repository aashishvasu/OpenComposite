#include "generated/interfaces/vrtypes.h"
#include "stdafx.h"
#include <vulkan/vulkan_core.h>

#if defined(SUPPORT_VK)

// Required for the close(2) call for the texture shared memory on Linux
#ifndef _WIN32
#include <unistd.h>
#endif

#include "../../DrvOpenXR/tmp_gfx/TemporaryVk.h"
#include "vkcompositor.h"

#include <vulkan/vulkan.h>

#define ERR(msg)                                                                                                                                         \
	do {                                                                                                                                                 \
		std::string str = "Hit Vulkan-related error " + string(msg) + " at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
		OOVR_ABORT(str.c_str());                                                                                                                         \
	} while (0)

static enum VkFormat handle_colorspace_auto(enum VkFormat ovrFormat)
{
	switch (ovrFormat) {
	// convert to SRGB
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		// Todo what games actually use these? and is color space always linear for these?
		return ovrFormat;
	default:
		OOVR_ABORTF("Unsupported texture format used: %d", ovrFormat);
	}
}

static enum VkFormat handle_colorspace_gamma(enum VkFormat ovrFormat)
{
	switch (ovrFormat) {
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		// Todo what games actually use these? and is color space always linear for these?
		return ovrFormat;
	default:
		OOVR_ABORTF("Unsupported texture format used: %d", ovrFormat);
	}
}

static enum VkFormat handle_colorspace_linear(enum VkFormat ovrFormat)
{
	switch (ovrFormat) {
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
		return VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		// Todo what games actually use these? and is color space always linear for these?
		return ovrFormat;
	default:
		OOVR_ABORTF("Unsupported texture format used: %d", ovrFormat);
	}
}

VkCompositor::VkCompositor(const vr::Texture_t* initialTexture)
{
	auto* tex = (vr::VRVulkanTextureData_t*)initialTexture->handle;

	appDevice = tex->m_pDevice;
	appQueue = tex->m_pQueue;

	VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	OOVR_FAILED_VK_ABORT(vkCreateCommandPool(tex->m_pDevice, &poolInfo, nullptr, &appCommandPool));
}

VkCompositor::~VkCompositor()
{
	// destroying command pool also frees command buffers
	vkDestroyCommandPool(appDevice, appCommandPool, nullptr);
}

void VkCompositor::Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds)
{
	const vr::VRVulkanTextureData_t* tex = (vr::VRVulkanTextureData_t*)texture->handle;

	if (!tex) {
		ERR("Cannot use NULL Vulkan image data (VRVulkanTextureData_t)");
	}

	// OpenXR only guarantees that the queue we initially gave it will have ownership of our swapchain image
	// We could safeguard against any insane applications with a queue that we manage and copy to it,
	// but it's simpler (and likely more performant) to just assume our app is sane.
	OOVR_FALSE_ABORT(appQueue == tex->m_pQueue);

	bool usable = chain != XR_NULL_HANDLE && CheckChainCompatible(*tex, createInfo, texture->eColorSpace);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		// First, delete the old chain if necessary
		if (chain)
			xrDestroySwapchain(chain);

		// Free old command buffers if necessary
		if (!appCommandBuffers.empty()) {
			vkFreeCommandBuffers(appDevice, appCommandPool, appCommandBuffers.size(), appCommandBuffers.data());
		}

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

		switch (texture->eColorSpace) {
		case vr::ColorSpace_Auto: {
			createInfo.format = handle_colorspace_auto((VkFormat)tex->m_nFormat);
			break;
		}
		case vr::ColorSpace_Gamma: {
			createInfo.format = handle_colorspace_gamma((VkFormat)tex->m_nFormat);
			break;
		}
		case vr::ColorSpace_Linear: {
			createInfo.format = handle_colorspace_linear((VkFormat)tex->m_nFormat);
			break;
		}
		}

		OOVR_FAILED_XR_ABORT(xrCreateSwapchain(xr_session.get(), &createInfo, &chain));

		uint32_t chainLength = 0;
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, 0, &chainLength, nullptr));
		swapchainImages.resize(chainLength);
		for (XrSwapchainImageVulkanKHR& swapchainImage : swapchainImages)
			swapchainImage.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, swapchainImages.size(), &chainLength, (XrSwapchainImageBaseHeader*)swapchainImages.data()));

		appCommandBuffers.resize(chainLength);
		VkCommandBufferAllocateInfo bufInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		bufInfo.commandPool = appCommandPool;
		bufInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufInfo.commandBufferCount = chainLength;
		OOVR_FAILED_VK_ABORT(vkAllocateCommandBuffers(appDevice, &bufInfo, appCommandBuffers.data()));
	}

	// First find the relevant image to render to
	XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t currentIndex;
	OOVR_FAILED_XR_ABORT(xrAcquireSwapchainImage(chain, &acquireInfo, &currentIndex));

	// Wait until the swapchain is ready - this makes sure the compositor isn't writing to it
	// We don't have to pass in currentIndex since it uses the oldest acquired-but-not-waited-on
	// image, so we should be careful with concurrency here.
	XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	OOVR_FAILED_XR_ABORT(xrWaitSwapchainImage(chain, &waitInfo));

	const VkCommandBuffer currentCommandBuffer = appCommandBuffers.at(currentIndex);
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	OOVR_FAILED_VK_ABORT(vkBeginCommandBuffer(currentCommandBuffer, &beginInfo));

	// transition swapchain image to TRANSFER_DST for copy
	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = swapchainImages.at(currentIndex).image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
	    currentCommandBuffer,
	    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &barrier);

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

	bool image_is_multisampled = xr_main_view(XruEyeLeft).maxSwapchainSampleCount < tex->m_nSampleCount;

	if (image_is_multisampled) {
		// HACK: As of July 2022 Monado does not support multisampling, so we can't just copy the image.
		// Instead, we do vkCmdResolveImage into the swapchain image. (note, this doesn't support depth textures)
		// Todo - how do we tell which runtimes support multisampling?

		vkCmdResolveImage( //
		    currentCommandBuffer, // commandbuffer
		    (VkImage)tex->m_nImage, // srcImage
		    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // srcImageLayout
		    swapchainImages.at(currentIndex).image, // dstImage
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // dstImageLayout
		    1, // regionCount
		    (VkImageResolve*)&region // pRegions
		);
	} else {
		vkCmdCopyImage( //
		    currentCommandBuffer, // commandbuffer
		    (VkImage)tex->m_nImage, // srcImage
		    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // srcImageLayout
		    swapchainImages.at(currentIndex).image, // dstImage
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // dstImageLayout
		    1, // regionCount
		    &region // pRegions
		);
	}

	// transition swapchain image back to COLOR_ATTACHMENT_OPTIMAL for runtime
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	vkCmdPipelineBarrier(
	    currentCommandBuffer, //
	    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &barrier);

	OOVR_FAILED_VK_ABORT(vkEndCommandBuffer(currentCommandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &currentCommandBuffer;

	OOVR_FAILED_VK_ABORT(vkQueueSubmit(tex->m_pQueue, 1, &submitInfo, VK_NULL_HANDLE));

	// Release the swapchain - OpenXR will use the last-released image in a swapchain
	XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	OOVR_FAILED_XR_ABORT(xrReleaseSwapchainImage(chain, &releaseInfo));
}

void VkCompositor::Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* ptrBounds, vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& layer)
{

	// Copy the texture across
	Invoke(texture, ptrBounds);

	vr::VRVulkanTextureData_t& tex = *(vr::VRVulkanTextureData_t*)texture->handle;

	// Set the viewport up
	XrSwapchainSubImage& subImage = layer.subImage;
	subImage.swapchain = chain;
	subImage.imageArrayIndex = 0; // This is *not* the swapchain index
	XrRect2Di& viewport = subImage.imageRect;

	// TODO deduplicate with dx11compositor, and use for all compositors
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		// We may have bounds.vMin > bounds.vMax representing a vertically flipped
		// image. Virtually all Vulkan implementations handle this (those supporting
		// VK_KHR_Maintenance1 and those supporting Vulkan 1.1) so this will
		// normally just work by default. TODO: detect version and either manually
		// flip or error out if not supported.

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
	switch (colourSpace) {
	case vr::ColorSpace_Auto: {
		if (handle_colorspace_auto((VkFormat)tex.m_nFormat) != chainDesc.format)
			FAIL(m_nFormat);
		break;
	}
	case vr::ColorSpace_Gamma: {
		if (handle_colorspace_gamma((VkFormat)tex.m_nFormat) != chainDesc.format)
			FAIL(m_nFormat);
		break;
	}
	case vr::ColorSpace_Linear: {
		if (handle_colorspace_linear((VkFormat)tex.m_nFormat) != chainDesc.format)
			FAIL(m_nFormat);
		break;
	}
	default:
		OOVR_ABORTF("Invalid colorspace given: %d", colourSpace);
		break;
	}

#undef CHECK
#undef FAIL

	return usable;
}
#endif
