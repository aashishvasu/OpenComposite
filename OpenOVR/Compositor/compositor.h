#pragma once

#include "OpenVR/vrtypes.h"
#include "d3dx12.h"

#include <Extras/OVR_Math.h>

#include <vector>

class Compositor {
public:
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds, vr::EVRSubmitFlags submitFlags) = 0;
protected:
	ovrTextureSwapChain *chains;
};

class DX12Compositor : public Compositor {
public:
	DX12Compositor(vr::D3D12TextureData_t *td, OVR::Sizei &bufferSize, ovrTextureSwapChain *chains);

	// Override
	virtual void Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds, vr::EVRSubmitFlags submitFlags);

private:
	ID3D12Device *device = NULL;
	ID3D12CommandQueue *queue = NULL;
	ID3D12GraphicsCommandList *commandList = NULL;

	int chainLength = -1;

	ID3D12CommandAllocator *allocator = NULL;
	ID3D12DescriptorHeap *rtvVRHeap = NULL;  // Resource Target View Heap
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> texRtv;
	std::vector<ID3D12Resource*> texResource;
};
