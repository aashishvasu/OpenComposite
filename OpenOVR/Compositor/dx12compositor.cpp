#include "stdafx.h"

#if defined(SUPPORT_DX) && defined(SUPPORT_DX12)

#include "dx12compositor.h"
#include "libovr_wrapper.h"

#include <string>

#include <atlbase.h>

#include "OVR_CAPI_D3D.h"

#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <d3d11.h>
#include "d3dx12.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")

using namespace vr;

std::string SHADER_STRING = R"HLSL(

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
	PSInput result;

	result.position = position;
	result.uv = uv;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return g_texture.Sample(g_sampler, input.uv);
}

)HLSL";

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

void ThrowIfCompileFailed(HRESULT test, ID3DBlob **error) {
	if ((test) != S_OK) {
		XTrace("Error compiling shader: %s\n", (char*)(*error)->GetBufferPointer());
		throw "ThrowIfFailed err";
	}
}

// TODO deleteme
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
};

DX12Compositor::DX12Compositor(D3D12TextureData_t *td, OVR::Sizei &bufferSize, ovrTextureSwapChain *chains) {
	this->chains = chains;
	singleScreenSize = bufferSize;

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

	D3D12_RESOURCE_DESC targetDesc = td->m_pResource->GetDesc();

	ovrTextureSwapChainDesc dsDesc = {};
	dsDesc.Type = ovrTexture_2D;
	dsDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	dsDesc.ArraySize = 1;
	dsDesc.Width = (int)targetDesc.Width;
	dsDesc.Height = (int)targetDesc.Height;
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
	if (ovr_CreateTextureSwapChainDX(session, queue.Get()/*xapp->reald3d11Device.Get()*/, &dsDesc, chain) != ovrSuccess) {
		throw "Could not create texture swap chain!";
	}

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
		ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvVRHeap)), device.Get());
		rtvVRHeap->SetName(L"rtVRHeap_xapp");

		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create the second chain,
	// but we don't have to set up everything like we did the first time
	if (ovr_CreateTextureSwapChainDX(session, queue.Get()/*xapp->reald3d11Device.Get()*/, &dsDesc, chains + 1) != ovrSuccess) {
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

	// TODO close command lsit
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(),
		nullptr, IID_PPV_ARGS(&commandList)), device.Get());
	commandList->Close(); // Close it so it's ready to be reset when ->Invoke is called

	// Create an empty root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3DBlob *signature;
		ID3DBlob *error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&signature, &error), device.Get());
		ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(),
			signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)), device.Get());
	}

	// Create descriptor heap.
	// For using textures in the shader:
	{
		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = count;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)), device.Get());

		m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ID3DBlob *vertexShader;
		ID3DBlob *pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ID3DBlob *error;
		ThrowIfCompileFailed(D3DCompile(SHADER_STRING.c_str(), SHADER_STRING.length(), "shader-compiled",
			nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &error), &error);
		ThrowIfCompileFailed(D3DCompile(SHADER_STRING.c_str(), SHADER_STRING.length(), "shader-compiled",
			nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &error), &error);

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)), device.Get());
	}

	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		Vertex triangleVertices[] =
		{
			// First triangle
			{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } }, // Bottom left corner
			{ { 1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f } }, // Top right corner
			{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 0.0f } }, // Top left corner

			// Second triangle
			{ { 1.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } }, // Bottom right corner
			{ { 1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f } }, // Top right corner
			{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } }, // Bottom left corner
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)),
			device.Get());

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)), device.Get());
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}
}

void DX12Compositor::Invoke(ovrEyeType eye, const Texture_t * texture, const VRTextureBounds_t * bounds,
	EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	// TODO without this, the eye images are reversed. What is the root cause of this?
	// TODO or is this a misinterpretation?
	eye = (ovrEyeType) (1 - eye);

	//if (true) return;

	// TODO support multisampling

	// Last frame's command list ought to be done by now?
	commandList->Reset(allocator.Get(), pipelineState.Get());

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	D3D12TextureData_t *input = (D3D12TextureData_t*)texture->handle;

	int currentIndex;
	ovr_GetTextureSwapChainCurrentIndex(*ovr::session, chains[eye], &currentIndex);
	int resId = eye * chainLength + currentIndex;
	XTrace("Rendering eye %d\n", resId);

	// TODO start

	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// TODO cache this value
	UINT srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // FIXME we're cheating by looking at the example - find this at runtime
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // TODO runtime check if using multisampling
	srvDesc.Texture2D.MipLevels = 1;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += srvDescriptorSize * resId;
	device->CreateShaderResourceView(input->m_pResource, &srvDesc, handle);

	// TODO destroy SRV when done

	commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());

	CD3DX12_VIEWPORT m_viewport(0.0, 0.0, (float)singleScreenSize.w, (float)singleScreenSize.h);
	CD3DX12_RECT m_scissorRect(0, 0, singleScreenSize.w, singleScreenSize.h);
	commandList->RSSetViewports(1, &m_viewport);
	commandList->RSSetScissorRects(1, &m_scissorRect);

	// Render into the target texture
	commandList->OMSetRenderTargets(1, &texRtv[resId], FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(texRtv[resId], clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->DrawInstanced(6, 1, 0, 0);

	// TODO end

	commandList->Close();

	ID3D12CommandList *set[] = { commandList.Get() };
	queue->ExecuteCommandLists(1, set);
}

#endif
