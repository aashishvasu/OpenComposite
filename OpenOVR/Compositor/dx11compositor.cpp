#include "stdafx.h"

#if defined(SUPPORT_DX) && defined(SUPPORT_DX11)

#include "dx11compositor.h"

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
		OOVR_MESSAGE(str.c_str(), "Errored func!");                                                                                                    \
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

void DX11Compositor::CheckCreateSwapChain(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, bool cube)
{
	XrSwapchainCreateInfo& desc = createInfo;

	auto* src = (ID3D11Texture2D*)texture->handle;

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	if (bounds)
	{
		if(std::fabs(bounds->uMax - bounds->uMin) > 0.1)
			srcDesc.Width = uint32_t(float(srcDesc.Width) * std::fabs(bounds->uMax - bounds->uMin));
		if (std::fabs(bounds->vMax - bounds->vMin) > 0.1)
			srcDesc.Height = uint32_t(float(srcDesc.Height) * std::fabs(bounds->vMax - bounds->vMin));
	}

	if (cube) {
		// LibOVR can only use square cubemaps, while SteamVR can use any shape
		// Note we use CopySubresourceRegion later on, so this won't cause problems with that
		srcDesc.Height = srcDesc.Width = min(srcDesc.Height, srcDesc.Width);
	}

	bool usable = chain == NULL ? false : CheckChainCompatible(srcDesc, texture->eColorSpace);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		if(bounds)
			OOVR_LOGF("Bounds: uMin %f uMax %f vMin %f vMax %f", bounds->uMin, bounds->uMax, bounds->vMin, bounds->vMax);
		OOVR_LOGF("Texture desc format: %d", srcDesc.Format);
		OOVR_LOGF("Texture desc bind flags: %d", srcDesc.BindFlags);
		OOVR_LOGF("Texture desc MiscFlags: %d", srcDesc.MiscFlags);
		OOVR_LOGF("Texture desc Usage: %d", srcDesc.Usage);
		OOVR_LOGF("Texture desc width: %d", srcDesc.Width);
		OOVR_LOGF("Texture desc height: %d", srcDesc.Height);

		// First, delete the old chain if necessary
		if (chain) {
			xrDestroySwapchain(chain);
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

		// TODO do we need to do anything wrt automatic mipmap generation?
		desc.usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;

		XrResult result = xrCreateSwapchain(xr_session, &desc, &chain);
		if (!XR_SUCCEEDED(result))
			OOVR_ABORTF("Cannot create DX texture swap chain: err %d", result);

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

void DX11Compositor::Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds)
{
	auto* src = (ID3D11Texture2D*)texture->handle;

	// OpenXR swap chain doesn't support weird formats like DXGI_FORMAT_BC1_TYPELESS
	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);
	if (srcDesc.Format == DXGI_FORMAT_BC1_TYPELESS) {
		if (chain) {
			xrDestroySwapchain(chain);
		}
		return;
	}
	
	CheckCreateSwapChain(texture, bounds, false);

	// First reserve an image from the swapchain
	XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t currentIndex = 0;
	OOVR_FAILED_XR_ABORT(xrAcquireSwapchainImage(chain, &acquireInfo, &currentIndex));

	// Wait until the swapchain is ready - this makes sure the compositor isn't writing to it
	// We don't have to pass in currentIndex since it uses the oldest acquired-but-not-waited-on
	// image, so we should be careful with concurrency here.
	// XR_TIMEOUT_EXPIRED is considered successful but swapchain still can't be used so need to handle that
	XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	waitInfo.timeout = 500000000; // time out in nano seconds - 500ms
	XrResult res;
	OOVR_FAILED_XR_ABORT(res = xrWaitSwapchainImage(chain, &waitInfo));

	if(res == XR_TIMEOUT_EXPIRED)
		OOVR_ABORTF("xrWaitSwapchainImage timeout");
		

	// Copy the source to the destination image
	D3D11_BOX sourceRegion;
	if (bounds) {
		sourceRegion.left = bounds->uMin == 0.0f ? 0 : createInfo.width;
		sourceRegion.right = bounds->uMin == 0.0f ? createInfo.width : createInfo.width * 2;
	}
	else {
		sourceRegion.left = 0;
		sourceRegion.right = createInfo.width;
	}
	sourceRegion.top = 0;
	sourceRegion.bottom = createInfo.height;
	sourceRegion.front = 0;
	sourceRegion.back = 1;
	
	context->CopySubresourceRegion(imagesHandles[currentIndex].texture, 0, 0, 0, 0, (ID3D11Texture2D*)texture->handle, 0, &sourceRegion);
		
	// Release the swapchain - OpenXR will use the last-released image in a swapchain
	XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	OOVR_FAILED_XR_ABORT(xrReleaseSwapchainImage(chain, &releaseInfo));
}

void DX11Compositor::InvokeCubemap(const vr::Texture_t* textures)
{
	CheckCreateSwapChain(&textures[0], nullptr, true);

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
	Invoke(texture, ptrBounds);

	// Set the viewport up
	// TODO deduplicate with dx11compositor, and use for all compositors
	XrSwapchainSubImage& subImage = layer.subImage;
	subImage.swapchain = chain;
	subImage.imageArrayIndex = 0; // This is *not* the swapchain index
	XrRect2Di& viewport = subImage.imageRect;
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
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

bool DX11Compositor::CheckChainCompatible(D3D11_TEXTURE2D_DESC& inputDesc, vr::EColorSpace colourSpace)
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

bool DX11Compositor::GetFormatInfo(DXGI_FORMAT format, DX11Compositor::DxgiFormatInfo& out)
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
