#include "stdafx.h"
#include "compositor.h"
#include "libovr_wrapper.h"

#include "OVR_CAPI_D3D.h"

using namespace std;
#define OVSS (*ovr::session)

static void XTrace(LPCSTR lpszFormat, ...) {
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnprintf_s(szBuffer, 511, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	OOVR_LOG(szBuffer);
	va_end(args);
}

ovrTextureFormat dxgiToOvrFormat(DXGI_FORMAT dxgi, vr::EColorSpace colourSpace) {
	// TODO is this really how it should work?
	// No idea why or how or what, but for now just force SRGB on as otherwise
	// it causes trouble.
	bool useSrgb = true; // colourSpace != vr::ColorSpace_Auto;

	switch (dxgi) {
#define MAPPING(name) \
			case DXGI_ ## name: \
				return OVR_ ## name;

#define C_MAPPING(name) \
			case DXGI_ ## name: \
			case DXGI_ ## name ## _SRGB: \
				return useSrgb ? OVR_ ## name ## _SRGB : OVR_ ## name;

		MAPPING(FORMAT_B5G6R5_UNORM);
		MAPPING(FORMAT_B5G5R5A1_UNORM);
		MAPPING(FORMAT_B4G4R4A4_UNORM);
		C_MAPPING(FORMAT_R8G8B8A8_UNORM);
		C_MAPPING(FORMAT_B8G8R8A8_UNORM);
		C_MAPPING(FORMAT_B8G8R8X8_UNORM);
		MAPPING(FORMAT_R16G16B16A16_FLOAT);
		MAPPING(FORMAT_R11G11B10_FLOAT);

	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return useSrgb ? OVR_FORMAT_R8G8B8A8_UNORM_SRGB : OVR_FORMAT_R8G8B8A8_UNORM;

#undef C_MAPPING
#undef MAPPING
	}

	return OVR_FORMAT_UNKNOWN;
}

#define ERR(msg) { \
	std::string str = "Hit DX11-related error " + string(msg) + " at " __FILE__ ":" + std::to_string(__LINE__) + " func " + std::string(__func__); \
	OOVR_LOG(str.c_str()); \
	MessageBoxA(NULL, str.c_str(), "Errored func!", MB_OK); \
	/**((int*)NULL) = 0;*/\
	throw str; \
}

void DX11Compositor::ThrowIfFailed(HRESULT test) {
	if ((test) != S_OK) {
		OOVR_FAILED_DX_ABORT(device->GetDeviceRemovedReason());
		throw "ThrowIfFailed err";
	}
}

DX11Compositor::DX11Compositor(ID3D11Texture2D *initial) {
	initial->GetDevice(&device);
	device->GetImmediateContext(&context);
}

DX11Compositor::~DX11Compositor() {
	context->Release();
	device->Release();
}

void DX11Compositor::Invoke(const vr::Texture_t * texture) {
	ovrTextureSwapChainDesc &desc = chainDesc;

	ID3D11Texture2D *src = (ID3D11Texture2D*)texture->handle;

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	bool usable = chain == NULL ? false : CheckChainCompatible(srcDesc, desc, texture->eColorSpace);

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
		desc.Format = dxgiToOvrFormat(srcDesc.Format, texture->eColorSpace);
		desc.MipLevels = srcDesc.MipLevels;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;

		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
		desc.BindFlags = ovrTextureBind_None; // ovrTextureBind_DX_RenderTarget;

		srcSize.w = srcDesc.Width;
		srcSize.h = srcDesc.Height;

		ovrResult result = ovr_CreateTextureSwapChainDX(OVSS, device, &desc, &chain);
		if (!OVR_SUCCESS(result))
			ERR("Cannot create DX texture swap chain " + to_string(result));
	}

	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(OVSS, chain, &currentIndex);

	ID3D11Texture2D* tex = nullptr;
	ovr_GetTextureSwapChainBufferDX(OVSS, chain, currentIndex, IID_PPV_ARGS(&tex));
	context->CopyResource(tex, src);
	tex->Release();
}

void DX11Compositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * ptrBounds,
	vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	// Copy the texture across
	Invoke(texture);

	// Set the viewport up
	ovrRecti &viewport = layer.Viewport[eye];
	if (ptrBounds) {
		vr::VRTextureBounds_t bounds = *ptrBounds;

		if (bounds.vMin > bounds.vMax) {
			submitVerticallyFlipped = true;
			float newMax = bounds.vMin;
			bounds.vMin = bounds.vMax;
			bounds.vMax = newMax;
		}
		else {
			submitVerticallyFlipped = false;
		}

		viewport.Pos.x = (int)(bounds.uMin * chainDesc.Width);
		viewport.Pos.y = (int)(bounds.vMin * chainDesc.Height);
		viewport.Size.w = (int)((bounds.uMax - bounds.uMin) * chainDesc.Width);
		viewport.Size.h = (int)((bounds.vMax - bounds.vMin) * chainDesc.Height);
	}
	else {
		viewport.Pos.x = viewport.Pos.y = 0;
		viewport.Size.w = chainDesc.Width;
		viewport.Size.h = chainDesc.Height;

		submitVerticallyFlipped = false;
	}
}

unsigned int DX11Compositor::GetFlags() {
	return submitVerticallyFlipped ? ovrLayerFlag_TextureOriginAtBottomLeft : 0;
}

bool DX11Compositor::CheckChainCompatible(D3D11_TEXTURE2D_DESC & inputDesc, ovrTextureSwapChainDesc &chainDesc, vr::EColorSpace colourSpace) {
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
	if(chainDesc.Format != dxgiToOvrFormat(inputDesc.Format, colourSpace)) FAIL(Format);
	//CHECK_ADV(SampleDesc.Count, SampleCount);
	//CHECK_ADV(SampleDesc.Quality);
#undef CHECK
#undef CHECK_ADV
#undef FAIL

	return usable;
}
