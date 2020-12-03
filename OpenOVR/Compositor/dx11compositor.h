#pragma once

#include "dxcompositor.h"

class DX11Compositor : public Compositor {
public:
	DX11Compositor(ID3D11Texture2D* td);

	virtual ~DX11Compositor() override;

	// Override
	virtual void Invoke(const vr::Texture_t* texture) override;

	virtual void InvokeCubemap(const vr::Texture_t* textures) override;
	virtual bool SupportsCubemap() override { return true; }

	virtual void Invoke(XruEye eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, XrCompositionLayerProjectionView& viewport) override;

	unsigned int GetFlags() override;

	ID3D11Device* GetDevice() { return device; }

protected:
	void CheckCreateSwapChain(const vr::Texture_t* texture, bool cube);

	void ThrowIfFailed(HRESULT test);

	bool CheckChainCompatible(D3D11_TEXTURE2D_DESC& inputDesc, vr::EColorSpace colourSpace);

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;

	bool submitVerticallyFlipped = false;

	std::vector<XrSwapchainImageD3D11KHR> imagesHandles;
};
