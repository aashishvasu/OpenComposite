#include "stdafx.h"
#include "compositor.h"
#include "libovr_wrapper.h"

#include <atlbase.h>

#include "OVR_CAPI_D3D.h"

using namespace vr;

void ThrowIfFailed(HRESULT test, ID3D12Device *device) {
	if ((test) != S_OK) {
		HRESULT remReason = device->GetDeviceRemovedReason();
		throw "ThrowIfFailed err";
	}
}

void XTrace(LPCSTR lpszFormat, ...) {
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnprintf_s(szBuffer, 511, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	va_end(args);
}

DX12Compositor::DX12Compositor(D3D12TextureData_t *td, OVR::Sizei &bufferSize, ovrTextureSwapChain *chains) {
	this->chains = chains;

	ovrSession &session = *ovr::session;
	queue = td->m_pCommandQueue;

	queue->GetDevice(IID_PPV_ARGS(&device));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));

	// xapp->d3d11Device.Get() will not work, we need a real D3D11 device

	//for (int i = 0; i < xapp->FrameCount; i++) {
	//	ID3D12Resource *resource = xapp->renderTargets[i].Get();
	//	D3D12_RESOURCE_DESC rDesc = resource->GetDesc();
	//	D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
	//	ThrowIfFailed(xapp->d3d11On12Device->CreateWrappedResource(
	//		resource,
	//		&d3d11Flags,
	//		D3D12_RESOURCE_STATE_RENDER_TARGET,
	//		D3D12_RESOURCE_STATE_PRESENT,
	//		IID_PPV_ARGS(&xapp->wrappedBackBuffers[i])
	//		));
	//	//xapp->d3d11On12Device->AcquireWrappedResources(xapp->wrappedBackBuffers[i].GetAddressOf(), 1);
	//}

	ovrTextureSwapChainDesc dsDesc = {};
	dsDesc.Type = ovrTexture_2D;
	dsDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	dsDesc.ArraySize = 1;
	dsDesc.Width = bufferSize.w;
	dsDesc.Height = bufferSize.h;
	dsDesc.MipLevels = 1;
	dsDesc.SampleCount = 1;
	dsDesc.StaticImage = ovrFalse;
	dsDesc.MiscFlags = ovrTextureMisc_DX_Typeless;//ovrTextureMisc_None;
	dsDesc.BindFlags = ovrTextureBind_DX_RenderTarget;

	/*	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = bufferSize.w;
	dsDesc.Height = bufferSize.h;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	*/
	ovrTextureSwapChain *chain = chains;
	if (ovr_CreateTextureSwapChainDX(session, queue/*xapp->reald3d11Device.Get()*/, &dsDesc, chain) == ovrSuccess) {
		int count = 0;
		ovr_GetTextureSwapChainLength(session, *chain, &chainLength);
		count = chainLength * 2; // Because two eyes
		texRtv.resize(count);
		texResource.resize(count);
		// Create descriptor heaps.
		UINT rtvDescriptorSize;
		{
			// Describe and create a render target view (RTV) descriptor heap.

			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = count;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvVRHeap)), device);
			rtvVRHeap->SetName(L"rtVRHeap_xapp");

			rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// Create the second chain,
		// but we don't have to set up everything like we did the first time
		if (ovr_CreateTextureSwapChainDX(session, queue/*xapp->reald3d11Device.Get()*/, &dsDesc, chains + 1) != ovrSuccess) {
			throw "Could not create second swapchain";
		}

		for (size_t eye = 0; eye < 2; eye++) {
			chain = chains + eye;

			for (int i = 0; i < chainLength; ++i) {
				int resId = eye * chainLength + i;

				ID3D11Texture2D* tex = nullptr;
				ovr_GetTextureSwapChainBufferDX(session, *chain, i, IID_PPV_ARGS(&texResource[resId]));
				//xapp->reald3d11Device.Get()->CreateRenderTargetView(tex, nullptr, &texRtv[i]);

				D3D12_RENDER_TARGET_VIEW_DESC rtvd = {};
				rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				rtvd.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvVRHeap->GetCPUDescriptorHandleForHeapStart(), resId, rtvDescriptorSize);
				texRtv[resId] = rtvHandle;
				device->CreateRenderTargetView(texResource[resId], /*nullptr*/&rtvd, texRtv[resId]);
				XTrace("Initializing texResource[%d]\n", resId);

				//ComPtr<IDXGIResource> dxgires;
				//tex->QueryInterface<IDXGIResource>(&dxgires);
				////Log("dxgires = " << dxgires.GetAddressOf() << endl);
				//HANDLE shHandle;
				//dxgires->GetSharedHandle(&shHandle);
				////Log("shared handle = " << shHandle << endl);
				//xapp->d3d11Device->OpenSharedResource(shHandle, IID_PPV_ARGS(&xapp->wrappedTextures[i]));
				//tex->Release();
			}
		}
	}

	// TODO close command lsit
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&commandList)), device);
	commandList->Close(); // Close it so it's ready to be reset when ->Invoke is called
}

void DX12Compositor::Invoke(ovrEyeType eye, const Texture_t * texture, const VRTextureBounds_t * bounds, EVRSubmitFlags submitFlags) {
	//if (true) return;

	// TODO support multisampling

	// Last frame's command list ought to be done by now?
	commandList->Reset(allocator, nullptr);

	D3D12TextureData_t *input = (D3D12TextureData_t*)texture->handle;

	int currentIndex;
	ovr_GetTextureSwapChainCurrentIndex(*ovr::session, chains[eye], &currentIndex);
	int resId = eye * chainLength + currentIndex;
	XTrace("Rendering eye %d\n", resId);

	CD3DX12_TEXTURE_COPY_LOCATION sourceInfo(input->m_pResource, UINT(0));
	CD3DX12_TEXTURE_COPY_LOCATION destInfo(texResource[resId], UINT(0));

	commandList->CopyTextureRegion(&destInfo, 0, 0, 0, &sourceInfo, nullptr);
	commandList->Close();

	ID3D12CommandList *set[] = { commandList };
	queue->ExecuteCommandLists(1, set);
}
