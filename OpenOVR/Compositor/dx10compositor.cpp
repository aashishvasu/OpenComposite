#include "stdafx.h"

#if defined(SUPPORT_DX) && defined(SUPPORT_DX10) && defined(SUPPORT_DX11)

#include "dx10compositor.h"

#pragma warning(push)
#pragma warning(disable : 4838)   // int to UINT truncation.
#include <atlbase.h>
#pragma warning(pop)

#include "OVR_CAPI_D3D.h"

static ID3D11Texture2D *tex10to11(ID3D10Texture2D *in) {
	OOVR_FALSE_ABORT(in != nullptr);

	CComQIPtr<ID3D11Texture2D> dx11Texture = in;
	OOVR_FALSE_ABORT(dx11Texture != nullptr);

	return dx11Texture;
}

DX10Compositor::DX10Compositor(ID3D10Texture2D *initial) : DX11Compositor(tex10to11(initial)) {
	device1 = device;
	OOVR_FALSE_ABORT(device1 != nullptr);

	context1 = context;
	OOVR_FALSE_ABORT(context1 != nullptr);

	UINT deviceFlags = device->GetCreationFlags();
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// If the device is single threaded, the context state must be too
	UINT stateFlags = 0;
	if (deviceFlags & D3D11_CREATE_DEVICE_SINGLETHREADED) {
		stateFlags |= D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED;
	}

	OOVR_FAILED_DX_ABORT(device1->CreateDeviceContextState(
		stateFlags,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		__uuidof(ID3D11Device1),
		nullptr,
		&customContextState));
}

void DX10Compositor::Invoke(const vr::Texture_t * texture) {
	CComQIPtr<ID3D11Texture2D> src = static_cast<IUnknown*>(texture->handle);
	OOVR_FALSE_ABORT(src != nullptr);

	vr::Texture_t tex = { 0 };
	tex.handle = src;
	tex.eColorSpace = texture->eColorSpace;
	tex.eType = texture->eType;
	DX11Compositor::Invoke(&tex);
}

void DX10Compositor::LoadSubmitContext() {
	context1->SwapDeviceContextState(customContextState, &originalContextState);
}

void DX10Compositor::ResetSubmitContext() {
	context1->SwapDeviceContextState(originalContextState, nullptr);
	originalContextState.Release();
}

#endif
