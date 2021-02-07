#pragma once

#include "dxcompositor.h"

class DX12Compositor : public Compositor {
public:
	DX12Compositor(vr::D3D12TextureData_t* td, OVR::Sizei& bufferSize, ovrTextureSwapChain* chains);

	// Override
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t* texture, const vr::VRTextureBounds_t* bounds,
	    vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov& layer) override;

private:
	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> queue;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	OVR::Sizei singleScreenSize;

	int chainLength = -1;

	ComPtr<ID3D12CommandAllocator> allocator = NULL;
	ComPtr<ID3D12DescriptorHeap> rtvVRHeap = NULL; // Resource Target View Heap
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> texRtv;
	std::vector<ID3D12Resource*> texResource;

	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12RootSignature> rootSignature;

	ComPtr<ID3D12DescriptorHeap> srvHeap = NULL;
	UINT m_rtvDescriptorSize;
};
