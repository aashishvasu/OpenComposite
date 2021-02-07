#pragma once

#include "dxcompositor.h"

#include "dx11compositor.h"

class DX10Compositor : public DX11Compositor {
public:
	DX10Compositor(ID3D10Texture2D* td);

	virtual void Invoke(const vr::Texture_t* texture) override;

	virtual void LoadSubmitContext() override;
	virtual void ResetSubmitContext() override;

private:
	CComQIPtr<ID3D11Device1> device1 = nullptr;
	CComQIPtr<ID3D11DeviceContext1> context1 = nullptr;
	CComPtr<ID3DDeviceContextState> customContextState = nullptr;
	CComPtr<ID3DDeviceContextState> originalContextState = nullptr;
};
