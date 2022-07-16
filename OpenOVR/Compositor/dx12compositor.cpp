#include "stdafx.h"

#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)

#include "dx12compositor.h"

#include <string>

#include <atlbase.h>


#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "d3dx12.h"
#include <d3d11.h>

#include "../Misc/Config.h"
#include "../Misc/xr_ext.h"


#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")

using namespace vr;
using namespace std;
using glm::min;

namespace {
	void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent)
	{
		if (fence->GetCompletedValue() < completionValue) {
			fence->SetEventOnCompletion(completionValue, waitEvent);
			WaitForSingleObject(waitEvent, INFINITE);
		}
	}
}


DX12Compositor::DX12Compositor(D3D12TextureData_t* td)
{
	queue = td->m_pCommandQueue;
	queue->GetDevice(IID_PPV_ARGS(&device));
}

DX12Compositor::~DX12Compositor()
{
	for (auto event : frameFenceEvents) {
		CloseHandle(event);
	}

	device->Release();
}

void DX12Compositor::CheckCreateSwapChain(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, bool cube)
{
	XrSwapchainCreateInfo& desc = createInfo;

	auto* src = (D3D12TextureData_t*)texture->handle;

	D3D12_RESOURCE_DESC srcDesc;
	srcDesc = src->m_pResource->GetDesc();

	if (bounds) {
		if (std::fabs(bounds->uMax - bounds->uMin) > 0.1)
			srcDesc.Width = uint32_t(float(srcDesc.Width) * std::fabs(bounds->uMax - bounds->uMin));
		if (std::fabs(bounds->vMax - bounds->vMin) > 0.1)
			srcDesc.Height = uint32_t(float(srcDesc.Height) * std::fabs(bounds->vMax - bounds->vMin));
	}

	if (cube) {
		// LibOVR can only use square cubemaps, while SteamVR can use any shape
		// Note we use CopySubresourceRegion later on, so this won't cause problems with that
		srcDesc.Height = srcDesc.Width = min(srcDesc.Height, (UINT)srcDesc.Width);
	}

	bool usable = chain == NULL ? false : CheckChainCompatible(srcDesc, texture->eColorSpace);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		if (bounds)
			OOVR_LOGF("Bounds: uMin %f uMax %f vMin %f vMax %f", bounds->uMin, bounds->uMax, bounds->vMin, bounds->vMax);
		OOVR_LOGF("Texture desc format: %d", srcDesc.Format);
		OOVR_LOGF("Texture desc flags:  %d", srcDesc.Flags);
		OOVR_LOGF("Texture desc width:  %d", srcDesc.Width);
		OOVR_LOGF("Texture desc height: %d", srcDesc.Height);

		// First, delete the old chain if necessary
		if (chain) {
			OOVR_FAILED_XR_ABORT(xrDestroySwapchain(chain));
			chain = XR_NULL_HANDLE;
		}

		// Figure out what format we need to use
		DxgiFormatInfo info = {};
		if (!GetFormatInfo(srcDesc.Format, info)) {
			OOVR_ABORTF("Unknown (by OC) DXGI texture format %d", srcDesc.Format);
		}
		bool useLinearFormat;
		switch (texture->eColorSpace) {
		case vr::ColorSpace_Gamma:
			useLinearFormat = false;
			break;
		case vr::ColorSpace_Linear:
			useLinearFormat = true;
			break;
		default:
			// As per the docs for the auto mode, at eight bits per channel or less it assumes gamma
			// (using such small channels for linear colour would result in significant banding)
			useLinearFormat = info.bpc > 8;
			break;
		}

		DXGI_FORMAT type = useLinearFormat ? info.linear : info.srgb;

		if (type == DXGI_FORMAT_UNKNOWN) {
			OOVR_ABORTF("Invalid DXGI target format found: useLinear=%d type=DXGI_FORMAT_UNKNOWN fmt=%d", useLinearFormat, srcDesc.Format);
		}

		// Set aside the old format for checking later
		createInfoFormat = srcDesc.Format;

		// Make eye render buffer
		desc = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		// TODO desc.Type = cube ? ovrTexture_Cube : ovrTexture_2D;
		desc.faceCount = cube ? 6 : 1;
		desc.width = srcDesc.Width;
		desc.height = srcDesc.Height;
		desc.format = type;
		desc.mipCount = srcDesc.MipLevels;
		desc.sampleCount = 1;
		desc.arraySize = 1;
		desc.usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

		XrResult result = xrCreateSwapchain(xr_session, &desc, &chain);
		if (!XR_SUCCEEDED(result))
			OOVR_ABORTF("Cannot create DX texture swap chain: err %d", result);

		// Go through the images and retrieve them - this will be used later in Invoke, since OpenXR doesn't
		// have a convenient way to request one specific image.
		uint32_t imageCount;
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, 0, &imageCount, nullptr));

		imagesHandles = std::vector<XrSwapchainImageD3D12KHR>(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR });
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain,
		    imagesHandles.size(), &imageCount, (XrSwapchainImageBaseHeader*)imagesHandles.data()));

		OOVR_FALSE_ABORT(imageCount == imagesHandles.size());

		for (auto event : frameFenceEvents) {
			CloseHandle(event);
		}

		commandLists.resize(imageCount);
		commandAllocators.resize(imageCount);
		frameFenceEvents.resize(imageCount);
		frameFences.resize(imageCount);
		fenceValues.resize(imageCount);

		for (uint32_t i = 0; i < imageCount; i++) {
			frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			fenceValues[i] = 0;
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
			    IID_PPV_ARGS(&frameFences[i]));

			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			    IID_PPV_ARGS(&commandAllocators[i]));
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
			    commandAllocators[i].Get(), nullptr,
			    IID_PPV_ARGS(&commandLists[i]));
			commandLists[i]->Close();
		}
		// TODO do we need to release the images at some point, or does the swapchain do that for us?
	}
}

void DX12Compositor::Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds)
{	
	D3D12TextureData_t* input = (D3D12TextureData_t*)texture->handle;

	// OpenXR swap chain doesn't support weird formats like DXGI_FORMAT_BC1_TYPELESS
	D3D12_RESOURCE_DESC srcDesc;
	srcDesc = input->m_pResource->GetDesc();
	if (srcDesc.Format == DXGI_FORMAT_BC1_TYPELESS) {
		if (chain) {
			OOVR_FAILED_XR_ABORT(xrDestroySwapchain(chain));
			chain = XR_NULL_HANDLE;
		}
		return;
	}

	CheckCreateSwapChain(texture, bounds, false);

	// First reserve an image from the swapchain
	XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t currentIndex = 0;
	OOVR_FAILED_XR_ABORT(xrAcquireSwapchainImage(chain, &acquireInfo, &currentIndex));

	WaitForFence(frameFences[currentIndex].Get(),
	    fenceValues[currentIndex], frameFenceEvents[currentIndex]);

	commandAllocators[currentIndex]->Reset();
	auto commandList = commandLists[currentIndex].Get();
	commandList->Reset(commandAllocators[currentIndex].Get(), nullptr);

	// Wait until the swapchain is ready - this makes sure the compositor isn't writing to it
	// We don't have to pass in currentIndex since it uses the oldest acquired-but-not-waited-on
	// image, so we should be careful with concurrency here.
	// XR_TIMEOUT_EXPIRED is considered successful but swapchain still can't be used so need to handle that
	XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	waitInfo.timeout = 500000000; // time out in nano seconds - 500ms
	XrResult res;
	OOVR_FAILED_XR_ABORT(res = xrWaitSwapchainImage(chain, &waitInfo));

	if (res == XR_TIMEOUT_EXPIRED)
		OOVR_ABORTF("xrWaitSwapchainImage timeout");

	// Copy the source to the destination image
	D3D12_BOX sourceRegion;
	if (bounds) {
		sourceRegion.left = bounds->uMin == 0.0f ? 0 : createInfo.width;
		sourceRegion.right = bounds->uMin == 0.0f ? createInfo.width : createInfo.width * 2;
	} else {
		sourceRegion.left = 0;
		sourceRegion.right = createInfo.width;
	}
	sourceRegion.top = 0;
	sourceRegion.bottom = createInfo.height;
	sourceRegion.front = 0;
	sourceRegion.back = 1;


	commandList->CopyResource(imagesHandles[currentIndex].texture, input->m_pResource);

	commandList->Close();	    
	ID3D12CommandList* set[] = { commandList };
	queue->ExecuteCommandLists(1, set);

	// Release the swapchain - OpenXR will use the last-released image in a swapchain
	XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	OOVR_FAILED_XR_ABORT(xrReleaseSwapchainImage(chain, &releaseInfo));

	const auto fenceValue = currentFenceValue;
	queue->Signal(frameFences[currentIndex].Get(), fenceValue);
	fenceValues[currentIndex] = fenceValue;
	++currentFenceValue;

}

void DX12Compositor::InvokeCubemap(const vr::Texture_t* textures)
{
	CheckCreateSwapChain(&textures[0], nullptr, true);

	OOVR_SOFT_ABORT("TODO cubemap");
}

void DX12Compositor::Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* ptrBounds,
    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& layer)
{

	// Copy the texture across
	Invoke(texture, ptrBounds);

	// Set the viewport up
	XrSwapchainSubImage& subImage = layer.subImage;
	subImage.swapchain = chain;
	subImage.imageArrayIndex = 0; // This is *not* the swapchain index
	XrRect2Di& viewport = subImage.imageRect;
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax && !oovr_global_configuration.InvertUsingShaders()) {
			std::swap(layer.fov.angleUp, layer.fov.angleDown);
			std::swap(bounds.vMin, bounds.vMax);
		}

		viewport.offset.x = 0;
		viewport.offset.y = 0;
		viewport.extent.width = createInfo.width;
		viewport.extent.height = createInfo.height;
	} else {
		viewport.offset.x = viewport.offset.y = 0;
		viewport.extent.width = createInfo.width;
		viewport.extent.height = createInfo.height;
	}
}

bool DX12Compositor::CheckChainCompatible(D3D12_RESOURCE_DESC& inputDesc, vr::EColorSpace colourSpace)
{
	bool usable = true;
#define FAIL(name)                             \
	do {                                       \
		usable = false;                        \
		OOVR_LOG("Resource mismatch: " #name); \
	} while (0);
#define CHECK(name, chainName)                  \
	if (inputDesc.name != createInfo.chainName) \
		FAIL(name);

	CHECK(Width, width)
	CHECK(Height, height)
	CHECK(MipLevels, mipCount)

	if (inputDesc.Format != createInfoFormat) {
		FAIL("Format");
	}

	// CHECK_ADV(SampleDesc.Count, SampleCount);
	// CHECK_ADV(SampleDesc.Quality);
#undef CHECK
#undef FAIL

	return usable;
}

bool DX12Compositor::GetFormatInfo(DXGI_FORMAT format, DX12Compositor::DxgiFormatInfo& out)
{
#define DEF_FMT_BASE(typeless, linear, srgb, bpp, bpc, channels)            \
	{                                                                       \
		out = DxgiFormatInfo{ srgb, linear, typeless, bpp, bpc, channels }; \
		return true;                                                        \
	}

#define DEF_FMT_NOSRGB(name, bpp, bpc, channels) \
	case name##_TYPELESS:                        \
	case name##_UNORM:                           \
		DEF_FMT_BASE(name##_TYPELESS, name##_UNORM, DXGI_FORMAT_UNKNOWN, bpp, bpc, channels)

#define DEF_FMT(name, bpp, bpc, channels) \
	case name##_TYPELESS:                 \
	case name##_UNORM:                    \
	case name##_UNORM_SRGB:               \
		DEF_FMT_BASE(name##_TYPELESS, name##_UNORM, name##_UNORM_SRGB, bpp, bpc, channels)

#define DEF_FMT_UNORM(linear, bpp, bpc, channels) \
	case linear:                                  \
		DEF_FMT_BASE(DXGI_FORMAT_UNKNOWN, linear, DXGI_FORMAT_UNKNOWN, bpp, bpc, channels)

	// Note that this *should* have pretty much all the types we'll ever see in games
	// Filtering out the non-typeless and non-unorm/srgb types, this is all we're left with
	// (note that types that are only typeless and don't have unorm/srgb variants are dropped too)
	switch (format) {
		// The relatively traditional 8bpp 32-bit types
		DEF_FMT(DXGI_FORMAT_R8G8B8A8, 32, 8, 4)
		DEF_FMT(DXGI_FORMAT_B8G8R8A8, 32, 8, 4)
		DEF_FMT(DXGI_FORMAT_B8G8R8X8, 32, 8, 3)

		// Some larger linear-only types
		DEF_FMT_NOSRGB(DXGI_FORMAT_R16G16B16A16, 64, 16, 4)
		DEF_FMT_NOSRGB(DXGI_FORMAT_R10G10B10A2, 32, 10, 4)

		// A jumble of other weird types
		DEF_FMT_UNORM(DXGI_FORMAT_B5G6R5_UNORM, 16, 5, 3)
		DEF_FMT_UNORM(DXGI_FORMAT_B5G5R5A1_UNORM, 16, 5, 4)
		DEF_FMT_UNORM(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM, 32, 10, 4)
		DEF_FMT_UNORM(DXGI_FORMAT_B4G4R4A4_UNORM, 16, 4, 4)
		DEF_FMT(DXGI_FORMAT_BC1, 64, 16, 4)

	default:
		// Unknown type
		return false;
	}

#undef DEF_FMT
#undef DEF_FMT_NOSRGB
#undef DEF_FMT_BASE
#undef DEF_FMT_UNORM
}

#endif
