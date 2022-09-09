#pragma once

#include "dxcompositor.h"

class DX11Compositor : public Compositor {
public:
	DX11Compositor(ID3D11Texture2D* td);

	virtual ~DX11Compositor() override;

	// Override
	virtual void Invoke(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds) override;

	virtual void InvokeCubemap(const vr::Texture_t* textures) override;
	virtual bool SupportsCubemap() override { return true; }

	virtual void Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& viewport) override;

	ID3D11Device* GetDevice() { return device; }

protected:
	void CheckCreateSwapChain(const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds, bool cube);

	void ThrowIfFailed(HRESULT test);

	bool CheckChainCompatible(D3D11_TEXTURE2D_DESC& inputDesc, vr::EColorSpace colourSpace);

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	ID3D11ShaderResourceView* quad_texture_view;
	ID3D11SamplerState* quad_sampleState;
	ID3D11VertexShader* fs_vshader;
	ID3D11PixelShader* fs_pshader;

	std::vector<XrSwapchainImageD3D11KHR> imagesHandles;
	std::vector<ID3D11RenderTargetView*> swapchain_rtvs;
	std::vector<ID3D11Texture2D*> resolvedMSAATextures;

	struct DxgiFormatInfo {
		/// The different versions of this format, set to DXGI_FORMAT_UNKNOWN if absent.
		/// Both the SRGB and linear formats should be UNORM.
		DXGI_FORMAT srgb, linear, typeless;

		/// THe bits per pixel, bits per channel, and the number of channels
		int bpp, bpc, channels;
	};

	/**
	 * Gets information about a given format into the output variable. Returns true if the texture was
	 * found, if not it returns false and leaves out in an undefined state.
	 */
	static bool GetFormatInfo(DXGI_FORMAT format, DxgiFormatInfo& out);
};
