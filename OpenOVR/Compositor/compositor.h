#pragma once

#include "OpenVR/interfaces/vrtypes.h"
#include "d3dx12.h"
#include <d3d11.h>
#include <wrl/client.h>

#include <Extras/OVR_Math.h>

#include <vector>
#include <memory>

using Microsoft::WRL::ComPtr;

typedef unsigned int GLuint;

class Compositor {
public:
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
		vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) = 0;

	virtual unsigned int GetFlags() { return 0; }
protected:
	ovrTextureSwapChain *chains;
	OVR::Sizei singleScreenSize;
};

class DX12Compositor : public Compositor {
public:
	DX12Compositor(vr::D3D12TextureData_t *td, OVR::Sizei &bufferSize, ovrTextureSwapChain *chains);

	// Override
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
		vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) override;

private:
	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> queue;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	int chainLength = -1;

	ComPtr<ID3D12CommandAllocator> allocator = NULL;
	ComPtr<ID3D12DescriptorHeap> rtvVRHeap = NULL;  // Resource Target View Heap
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> texRtv;
	std::vector<ID3D12Resource*> texResource;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12RootSignature> rootSignature;

	ComPtr<ID3D12DescriptorHeap> srvHeap = NULL;
	UINT m_rtvDescriptorSize;
};

class DX11Compositor : public Compositor {
public:
	DX11Compositor(ID3D11Texture2D* td, OVR::Sizei bufferSize, ovrTextureSwapChain *chains);

	// Override
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
		vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) override;

private:
	void ThrowIfFailed(HRESULT test);

	bool CheckChainCompatible(D3D11_TEXTURE2D_DESC & inputDesc, ovrTextureSwapChainDesc & chainDesc);

	ID3D11Device *device;
	ID3D11DeviceContext *context;

	ovrTextureSwapChainDesc chainDescs[2];
};

class GLCompositor : public Compositor {
public:
	GLCompositor(ovrTextureSwapChain *chains, OVR::Sizei size);

	unsigned int GetFlags() override;

	// Override
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
		vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) override;

private:
	GLuint fboId;
};
