#pragma once

#include "generated/interfaces/vrtypes.h"
#ifdef WIN32
// Windows template libraries
#include <atlbase.h>
#include <wrl/client.h>
#endif

#include "../Misc/xr_ext.h"
#include "../Misc/xrutil.h"

#include <memory>
#include <optional>
#include <vector>

#ifdef WIN32
using Microsoft::WRL::ComPtr;
#endif

typedef unsigned int GLuint;

class Compositor {
public:
	virtual ~Compositor();

	void Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, XrSwapchainSubImage& subImage, std::optional<XruEye> eye = std::nullopt, vr::EVRSubmitFlags submitFlags = vr::Submit_Default);
	bool CalculateViewport(const vr::VRTextureBounds_t* bounds, int32_t width, int32_t height, bool supportsInvert, XrRect2Di& viewport);

	virtual void InvokeCubemap(const vr::Texture_t* textures) = 0;
	virtual bool SupportsCubemap() { return false; }

	virtual XrSwapchain GetSwapChain() { return chain; };

	virtual XrExtent2Di GetSrcSize() { return { static_cast<int32_t>(createInfo.width), static_cast<int32_t>(createInfo.height) }; }
	/**
	 * Loads and unloads some context required for submitting textures to LibOVR. LoadSubmitContext is
	 *  called before calling either Invoke or ovr_CommitTextureSwapChain, and ResetSubmitContext after
	 *  calling both of them.
	 */
	virtual void LoadSubmitContext(){};
	virtual void ResetSubmitContext(){};

protected:
	virtual void CopyToSwapchain(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, std::optional<XruEye> eye, vr::EVRSubmitFlags submitFlags) = 0;

	XrSwapchain chain = XR_NULL_HANDLE;

	// The request used to create the current swapchain. This can be used to check if the swapchain needs recreating.
	XrSwapchainCreateInfo createInfo{};

	// The format specified by the game when creating the swapchain. This is used for verifying the format hasn't changed, since
	// we do fiddle with it a bit to get the SRGB stuff done correctly.
	int64_t createInfoFormat;
};
