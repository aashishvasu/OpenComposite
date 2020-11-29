#include "stdafx.h"

#include "compositor.h"
#include "dxcompositor.h"

#include "../Misc/xr_ext.h"

using namespace std;
using glm::min;

static void XTrace(LPCSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnprintf_s(szBuffer, 511, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	OOVR_LOG(szBuffer);
	va_end(args);
}

#define ERR(msg)                                                                                                                                       \
	{                                                                                                                                                  \
		std::string str = "Hit DX11-related error " + string(msg) + " at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
		OOVR_LOG(str.c_str());                                                                                                                         \
		MessageBoxA(NULL, str.c_str(), "Errored func!", MB_OK);                                                                                        \
		/**((int*)NULL) = 0;*/                                                                                                                         \
		throw str;                                                                                                                                     \
	}

void DX11Compositor::ThrowIfFailed(HRESULT test)
{
	if ((test) != S_OK) {
		OOVR_FAILED_DX_ABORT(device->GetDeviceRemovedReason());
		throw "ThrowIfFailed err";
	}
}

DX11Compositor::DX11Compositor(ID3D11Texture2D* initial)
{
	initial->GetDevice(&device);
	device->GetImmediateContext(&context);
}

DX11Compositor::~DX11Compositor()
{
	context->Release();
	device->Release();
}

void DX11Compositor::CheckCreateSwapChain(const vr::Texture_t* texture, bool cube)
{
	XrSwapchainCreateInfo& desc = createInfo;

	auto* src = (ID3D11Texture2D*)texture->handle;

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	if (cube) {
		// LibOVR can only use square cubemaps, while SteamVR can use any shape
		// Note we use CopySubresourceRegion later on, so this won't cause problems with that
		srcDesc.Height = srcDesc.Width = min(srcDesc.Height, srcDesc.Width);
	}

	bool usable = chain == NULL ? false : CheckChainCompatible(srcDesc, texture->eColorSpace);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		// First, delete the old chain if necessary
		if (chain) {
			xrDestroySwapchain(chain);
		}

		// Make eye render buffer
		desc = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		// TODO desc.Type = cube ? ovrTexture_Cube : ovrTexture_2D;
		desc.faceCount = cube ? 6 : 1;
		desc.width = srcDesc.Width;
		desc.height = srcDesc.Height;
		desc.format = srcDesc.Format; // TODO colourspace handling?
		desc.mipCount = srcDesc.MipLevels;
		desc.sampleCount = 1;
		desc.arraySize = 1;

		// TODO do we need to do anything wrt automatic mipmap generation?
		desc.usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;

		XrResult result = xrCreateSwapchain(xr_session, &desc, &chain);
		if (!XR_SUCCEEDED(result))
			ERR("Cannot create DX texture swap chain " + to_string(result));

		// Go through the images and retrieve them - this will be used later in Invoke, since OpenXR doesn't
		// have a convenient way to request one specific image.
		uint32_t imageCount;
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain, 0, &imageCount, nullptr));

		imagesHandles = std::vector<XrSwapchainImageD3D11KHR>(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
		OOVR_FAILED_XR_ABORT(xrEnumerateSwapchainImages(chain,
		    imagesHandles.size(), &imageCount, (XrSwapchainImageBaseHeader*)imagesHandles.data()));

		OOVR_FALSE_ABORT(imageCount == imagesHandles.size());

		// TODO do we need to release the images at some point, or does the swapchain do that for us?
	}
}

void DX11Compositor::Invoke(const vr::Texture_t* texture)
{
	CheckCreateSwapChain(texture, false);

	// First reserve an image from the swapchain
	XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t currentIndex = 0;
	OOVR_FAILED_XR_ABORT(xrAcquireSwapchainImage(chain, &acquireInfo, &currentIndex));

	// Wait until the swapchain is ready - this makes sure the compositor isn't writing to it
	// We don't have to pass in currentIndex since it uses the oldest acquired-but-not-waited-on
	// image, so we should be careful with concurrency here.
	XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	OOVR_FAILED_XR_ABORT(xrWaitSwapchainImage(chain, &waitInfo));

	// Copy the source to the destination image
	context->CopyResource(imagesHandles[currentIndex].texture, (ID3D11Texture2D*)texture->handle);

	// Release the swapchain - OpenXR will use the last-released image in a swapchain
	XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	OOVR_FAILED_XR_ABORT(xrReleaseSwapchainImage(chain, &releaseInfo))
}

void DX11Compositor::InvokeCubemap(const vr::Texture_t* textures)
{
	CheckCreateSwapChain(&textures[0], true);

#ifdef OC_XR_PORT
	ID3D11Texture2D* tex = nullptr;
	ERR("TODO cubemap");
#else
	int currentIndex = 0;
	OOVR_FAILED_OVR_ABORT(ovr_GetTextureSwapChainCurrentIndex(OVSS, chain, &currentIndex));

	OOVR_FAILED_OVR_ABORT(ovr_GetTextureSwapChainBufferDX(OVSS, chain, currentIndex, IID_PPV_ARGS(&tex)));
#endif

	ID3D11Texture2D* faceSrc;

	// Front
	faceSrc = (ID3D11Texture2D*)textures[0].handle;
	context->CopySubresourceRegion(tex, 5, 0, 0, 0, faceSrc, 0, nullptr);

	// Back
	faceSrc = (ID3D11Texture2D*)textures[1].handle;
	context->CopySubresourceRegion(tex, 4, 0, 0, 0, faceSrc, 0, nullptr);

	// Left
	faceSrc = (ID3D11Texture2D*)textures[2].handle;
	context->CopySubresourceRegion(tex, 0, 0, 0, 0, faceSrc, 0, nullptr);

	// Right
	faceSrc = (ID3D11Texture2D*)textures[3].handle;
	context->CopySubresourceRegion(tex, 1, 0, 0, 0, faceSrc, 0, nullptr);

	// Top
	faceSrc = (ID3D11Texture2D*)textures[4].handle;
	context->CopySubresourceRegion(tex, 2, 0, 0, 0, faceSrc, 0, nullptr);

	// Bottom
	faceSrc = (ID3D11Texture2D*)textures[5].handle;
	context->CopySubresourceRegion(tex, 3, 0, 0, 0, faceSrc, 0, nullptr);

	tex->Release();
}

void DX11Compositor::Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* ptrBounds,
    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& layer)
{

	// Copy the texture across
	Invoke(texture);

	// Set the viewport up
	// TODO deduplicate with dx11compositor, and use for all compositors
	XrSwapchainSubImage& subImage = layer.subImage;
	subImage.swapchain = chain;
	subImage.imageArrayIndex = 0; // This is *not* the swapchain index
	XrRect2Di& viewport = subImage.imageRect;
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
			submitVerticallyFlipped = true;
			float newMax = bounds.vMin;
			bounds.vMin = bounds.vMax;
			bounds.vMax = newMax;
		} else {
			submitVerticallyFlipped = false;
		}

		viewport.offset.x = (int)(bounds.uMin * createInfo.width);
		viewport.offset.y = (int)(bounds.vMin * createInfo.height);
		viewport.extent.width = (int)((bounds.uMax - bounds.uMin) * createInfo.width);
		viewport.extent.height = (int)((bounds.vMax - bounds.vMin) * createInfo.height);
	} else {
		viewport.offset.x = viewport.offset.y = 0;
		viewport.extent.width = createInfo.width;
		viewport.extent.height = createInfo.height;

		submitVerticallyFlipped = false;
	}
}

unsigned int DX11Compositor::GetFlags()
{
	ERR("TODO");
	// return submitVerticallyFlipped ? ovrLayerFlag_TextureOriginAtBottomLeft : 0;
}

bool DX11Compositor::CheckChainCompatible(D3D11_TEXTURE2D_DESC& inputDesc, vr::EColorSpace colourSpace)
{
	bool usable = true;
#define FAIL(name)                             \
	{                                          \
		usable = false;                        \
		OOVR_LOG("Resource mismatch: " #name); \
	}
#define CHECK(name, chainName)                  \
	if (inputDesc.name != createInfo.chainName) \
		FAIL(name);

	CHECK(Width, width)
	CHECK(Height, height)
	CHECK(MipLevels, mipCount)
	CHECK(Format, format)
	//CHECK_ADV(SampleDesc.Count, SampleCount);
	//CHECK_ADV(SampleDesc.Quality);
#undef CHECK
#undef FAIL

	return usable;
}
