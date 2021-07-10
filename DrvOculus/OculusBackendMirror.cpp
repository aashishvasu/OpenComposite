#include "DrvOculusCommon.h"
#include "OculusBackend.h"

#include "OVR_CAPI.h"
#include "OVR_CAPI_D3D.h"
#include "../OpenOVR/libovr_wrapper.h" // TODO refractor

#define SESS (*ovr::session)

// D3D mirror
// #if defined(SUPPORT_DX) // TODO reenable ifdefs
IBackend::openvr_enum_t OculusBackend::GetMirrorTextureD3D11(vr::EVREye eEye, void * pD3D11DeviceOrResource, void ** ppD3D11ShaderResourceView) {
	IUnknown *ukn = (IUnknown*)pD3D11DeviceOrResource;
	CComQIPtr<ID3D11Device> dev = ukn;

	if (!dev) {
		// GetMirrorTextureD3D11 accepts either a device or a resource
		// If it's a resource, grab it's device and hope we're not supposed to copy
		// the mirror onto the resource (as per usual, there is no documentation for this method).
		CComQIPtr<ID3D11Resource> resource = ukn;

		if (!resource) {
			OOVR_ABORT("Mirror texture arg2: does not implement ID3D11Device or ID3D11Resource!");
		}

		resource->GetDevice(&dev);
	}

	// For now, just ignore the eye since that would be a huge amount more work

	if (!mirrorTexture) {
		ovrMirrorTextureDesc mirrorDesc = {};
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = 1024; // TODO what resolution does SteamVR use?
		mirrorDesc.Height = 768;
		mirrorDesc.MirrorOptions = ovrMirrorOption_RightEyeOnly;
		OOVR_FAILED_OVR_ABORT(ovr_CreateMirrorTextureWithOptionsDX(SESS, dev, &mirrorDesc, &mirrorTexture));
	}

	CComPtr<ID3D11Texture2D> tex = nullptr;
	OOVR_FAILED_OVR_ABORT(ovr_GetMirrorTextureBufferDX(SESS, mirrorTexture, IID_PPV_ARGS(&tex)));

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;

	ID3D11ShaderResourceView *srv;
	OOVR_FAILED_DX_ABORT(dev->CreateShaderResourceView(tex, &desc, &srv));

	*ppD3D11ShaderResourceView = srv;

	mirrorTexturesCount++;

	return VRCompositorError_None;
}

void OculusBackend::ReleaseMirrorTextureD3D11(void * pD3D11ShaderResourceView) {
	ID3D11ShaderResourceView *srv = (ID3D11ShaderResourceView*)pD3D11ShaderResourceView;
	srv->Release();

	mirrorTexturesCount--;

	if (mirrorTexturesCount <= 0)
		DestroyOculusMirrorTexture();
}

void OculusBackend::DestroyOculusMirrorTexture() {
	// Destroy the texture when done with it.
	ovr_DestroyMirrorTexture(SESS, mirrorTexture);
	mirrorTexture = nullptr;
	mirrorTexturesCount = 0;
}
// #endif
