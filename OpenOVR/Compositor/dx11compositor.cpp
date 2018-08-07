#include "stdafx.h"
#include "compositor.h"
#include "libovr_wrapper.h"

#include "OVR_CAPI_D3D.h"

using namespace std;
#define OVSS (*ovr::session)

ovrTextureFormat dxgiToOvrFormat(DXGI_FORMAT dxgi) {
	switch (dxgi) {
#define MAPPING(name) \
			case DXGI_ ## name: \
				return OVR_ ## name;

		MAPPING(FORMAT_B5G6R5_UNORM);
		MAPPING(FORMAT_B5G5R5A1_UNORM);
		MAPPING(FORMAT_B4G4R4A4_UNORM);
		MAPPING(FORMAT_R8G8B8A8_UNORM);
		MAPPING(FORMAT_R8G8B8A8_UNORM_SRGB);
		MAPPING(FORMAT_B8G8R8A8_UNORM);
		MAPPING(FORMAT_B8G8R8A8_UNORM_SRGB);
		MAPPING(FORMAT_B8G8R8X8_UNORM);
		MAPPING(FORMAT_B8G8R8X8_UNORM_SRGB);
		MAPPING(FORMAT_R16G16B16A16_FLOAT);
		MAPPING(FORMAT_R11G11B10_FLOAT);

#undef MAPPING
	}

	return OVR_FORMAT_UNKNOWN;
}

void DX11Compositor::ThrowIfFailed(HRESULT test) {
	if ((test) != S_OK) {
		HRESULT remReason = device->GetDeviceRemovedReason();
		throw "ThrowIfFailed err";
	}
}

DX11Compositor::DX11Compositor(ID3D11Texture2D *initial, OVR::Sizei size, ovrTextureSwapChain *chains) {
	this->chains = chains;

	initial->GetDevice(&device);
	device->GetImmediateContext(&context); // TODO cleanup - copyContext->Release()
}

void DX11Compositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
	vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	ovrTextureSwapChain &chain = chains[eye];
	ovrTextureSwapChainDesc &desc = chainDescs[eye];

	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(OVSS, chain, &currentIndex);

	ID3D11Texture2D *src = (ID3D11Texture2D*)texture->handle;

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	bool usable = chain == NULL ? false : CheckChainCompatible(srcDesc, desc);

	if (!usable) {
		OOVR_LOG("Generating new swap chain");

		// First, delete the old chain if necessary
		if (chain)
			ovr_DestroyTextureSwapChain(OVSS, chain);

		// Make eye render buffer
		desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = srcDesc.Width;
		desc.Height = srcDesc.Height;
		desc.Format = dxgiToOvrFormat(srcDesc.Format);
		desc.MipLevels = srcDesc.MipLevels;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;

		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
		desc.BindFlags = ovrTextureBind_DX_RenderTarget;

		ovrResult result = ovr_CreateTextureSwapChainDX(OVSS, device, &desc, &chain);
		if (!OVR_SUCCESS(result))
			throw string("Cannot create GL texture swap chain");
	}

	ID3D11Texture2D* tex = nullptr;
	ovr_GetTextureSwapChainBufferDX(OVSS, chain, currentIndex, IID_PPV_ARGS(&tex));
	context->CopyResource(tex, src);

	// Set the viewport up
	ovrRecti &viewport = layer.Viewport[eye];
	if (bounds) {
		viewport.Pos.x = (int)(bounds->uMin * srcDesc.Width);
		viewport.Pos.y = (int)(bounds->vMin * srcDesc.Height);
		viewport.Size.w = (int)((bounds->uMax - bounds->uMin) * srcDesc.Width);
		viewport.Size.h = (int)((bounds->vMax - bounds->vMin) * srcDesc.Height);
	}
	else {
		viewport.Pos.x = viewport.Pos.y = 0;
		viewport.Size.w = srcDesc.Width;
		viewport.Size.h = srcDesc.Height;
	}
}

bool DX11Compositor::CheckChainCompatible(D3D11_TEXTURE2D_DESC & inputDesc, ovrTextureSwapChainDesc &chainDesc) {
	bool usable = true;
#define FAIL(name) { \
	usable = false; \
	OOVR_LOG("Resource mismatch: " #name); \
}
#define CHECK(name) CHECK_ADV(name, name)
#define CHECK_ADV(name, chainName) \
if(inputDesc.name != chainDesc.chainName) FAIL(name);

	CHECK(Width);
	CHECK(Height);
	CHECK(MipLevels);
	if(chainDesc.Format != dxgiToOvrFormat(inputDesc.Format)) FAIL(Format);
	//CHECK_ADV(SampleDesc.Count, SampleCount);
	//CHECK_ADV(SampleDesc.Quality);
#undef CHECK
#undef CHECK_ADV
#undef FAIL

	return usable;
}
