#pragma once

#include "dxcompositor.h"

class DX12Compositor : public Compositor {
public:
	DX12Compositor(vr::D3D12TextureData_t* td);

	// Override
	virtual ~DX12Compositor() override;

	// Override
	virtual void Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds) override;

	virtual void InvokeCubemap(const vr::Texture_t* textures) override;
	virtual bool SupportsCubemap() override { return false; }

	virtual void Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& viewport) override;

	ComPtr<ID3D12Device> GetDevice() { return device; }

private:
	void CheckCreateSwapChain(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, bool cube);

	void ThrowIfFailed(HRESULT test);

	bool CheckChainCompatible(D3D12_RESOURCE_DESC& inputDesc, vr::EColorSpace colourSpace);

	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> queue;

	std::vector<ComPtr<ID3D12CommandAllocator>> commandAllocators;
	std::vector<ComPtr<ID3D12GraphicsCommandList>> commandLists;

	std::vector < HANDLE > frameFenceEvents;
	std::vector < ComPtr<ID3D12Fence> > frameFences;
	std::vector < UINT64 > fenceValues;
	UINT64 currentFenceValue;

	std::vector<XrSwapchainImageD3D12KHR> imagesHandles;

	struct DxgiFormatInfo {
		/// The different versions of this format, set to DXGI_FORMAT_UNKNOWN if absent.
		/// Both the SRGB and linear formats should be UNORM.
		DXGI_FORMAT srgb, linear, typeless;

		/// The bits per pixel, bits per channel, and the number of channels
		int bpp, bpc, channels;
	};

	/**
	 * Gets information about a given format into the output variable. Returns true if the texture was
	 * found, if not it returns false and leaves out in an undefined state.
	 */
	static bool GetFormatInfo(DXGI_FORMAT format, DxgiFormatInfo& out);
};
