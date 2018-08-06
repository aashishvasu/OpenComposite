#include "stdafx.h"
#include "compositor.h"
#include "libovr_wrapper.h"

#include "OVR_CAPI_D3D.h"
#include <D3DCompiler.h>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

struct TEXPOINT { FLOAT X, Y, Z; XMFLOAT2 Color; };

static std::string SHADER_STRING = R"HLSL(

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VShader( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
    output.Tex = input.Tex;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PShader( PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample( samLinear, input.Tex );
}

)HLSL";

#define OVSS (*ovr::session)

void DX11Compositor::ThrowIfFailed(HRESULT test) {
	if ((test) != S_OK) {
		HRESULT remReason = device->GetDeviceRemovedReason();
		throw "ThrowIfFailed err";
	}
}

static void XTrace(LPCSTR lpszFormat, ...) {
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	char szBuffer[512]; // get rid of this hard-coded buffer
	nBuf = _vsnprintf_s(szBuffer, 511, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	va_end(args);
}

static void ThrowIfCompileFailed(HRESULT test, ID3DBlob **error) {
	if ((test) != S_OK) {
		XTrace("Error compiling shader: %s\n", (char*)(*error)->GetBufferPointer());
		throw "ThrowIfFailed err";
	}
}

DX11Compositor::DX11Compositor(ID3D11Texture2D *initial, OVR::Sizei size, ovrTextureSwapChain *chains) {
	this->chains = chains;

	initial->GetDevice(&device);
	device->GetImmediateContext(&context); // TODO cleanup - copyContext->Release()

	// Make eye render buffers
	for (int ieye = 0; ieye < 2; ++ieye) {
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = size.w;
		desc.Height = size.h;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;

		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
		desc.BindFlags = ovrTextureBind_DX_RenderTarget;

		ovrResult result = ovr_CreateTextureSwapChainDX(OVSS, device, &desc, &chains[ieye]);
		if (!OVR_SUCCESS(result))
			throw string("Cannot create GL texture swap chain");

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(OVSS, chains[ieye], &textureCount);
		for (int i = 0; i < textureCount; ++i) {
			ID3D11Texture2D* tex = nullptr;
			ovr_GetTextureSwapChainBufferDX(OVSS, chains[ieye], i, IID_PPV_ARGS(&tex));
			D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
			rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvd.ViewDimension = (desc.SampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			ID3D11RenderTargetView* rtv;
			ThrowIfFailed(device->CreateRenderTargetView(tex, &rtvd, &rtv));
			renderTargets[ieye].push_back(rtv);
			tex->Release();
		}
	}

	// load and compile the two shaders
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ID3DBlob *VS, *PS;

	ID3DBlob *error = nullptr;
	ThrowIfCompileFailed(
		D3DCompile(SHADER_STRING.c_str(), SHADER_STRING.length(), "shader-src-embedded", NULL, NULL,
			"VShader", "vs_5_0", compileFlags, 0, &VS, &error),
		&error);
	ThrowIfCompileFailed(
		D3DCompile(SHADER_STRING.c_str(), SHADER_STRING.length(), "shader-src-embedded", NULL, NULL,
			"PShader", "ps_5_0", compileFlags, 0, &PS, &error),
		&error);

	// encapsulate both shaders into shader objects
	ThrowIfFailed(device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS));
	ThrowIfFailed(device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS));

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ThrowIfFailed(device->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout));
	context->IASetInputLayout(pLayout);

	// Make the quad render geometry
	// create a triangle using the VERTEX struct
	// Note the above positions are flipped and I can't be bothered to change them
	TEXPOINT OurVertices[] =
	{
		// Left-top triangle
		{ -1.0f,  1.0f, 0.0f, XMFLOAT2(0, 1) }, // top left
		{  1.0f, -1.0f, 0.0f, XMFLOAT2(1, 0) }, // bottom right
		{ -1.0f, -1.0f, 0.0f, XMFLOAT2(0, 0) }, // bottom left

		// Lower-right triangle
		{ -1.0f,  1.0f, 0.0f, XMFLOAT2(0, 1) }, // top left
		{  1.0f,  1.0f, 0.0f, XMFLOAT2(1, 1) }, // top right
		{  1.0f, -1.0f, 0.0f, XMFLOAT2(1, 0) }, // bottom right
	};

	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(TEXPOINT) * 6;             // size is the VERTEX struct * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	ThrowIfFailed(device->CreateBuffer(&bd, NULL, &pVBuffer));       // create the buffer

												   // copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	ThrowIfFailed(context->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms));    // map the buffer
	memcpy(ms.pData, OurVertices, sizeof(OurVertices));                 // copy the data
	context->Unmap(pVBuffer, NULL);                                      // unmap the buffer
}

void DX11Compositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
	vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	ovrTextureSwapChain tex = chains[eye];

	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(OVSS, tex, &currentIndex);

	//ID3D11Texture2D* dest = nullptr;
	//ovr_GetTextureSwapChainBufferDX(OVSS, tex, currentIndex, IID_PPV_ARGS(&dest));

	//assert(texture->eType == TextureType_DirectX);

	ID3D11Texture2D *src = (ID3D11Texture2D*)texture->handle;

	//context->CopySubresourceRegion(dest, 0, 0, 0, 0, src, 0, NULL);

	// Wrap up our input texture into a shader resource view
	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	memset(&srvDesc, 0, sizeof(srvDesc));
	srvDesc.Format = srcDesc.Format;
	srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	ID3D11ShaderResourceView *srv;
	ThrowIfFailed(device->CreateShaderResourceView(src, &srvDesc, &srv));

	// Produce the final image
	ID3D11RenderTargetView* view = renderTargets[eye][currentIndex];

	// Set the render target to be the render to texture.
	context->OMSetRenderTargets(1, &view, NULL);

	// set the shader objects
	context->VSSetShader(pVS, 0, 0);
	context->PSSetShader(pPS, 0, 0);
	context->PSSetShaderResources(0, 1, &srv);

	// clear the back buffer to a deep blue
	FLOAT col[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	context->ClearRenderTargetView(view, col);

	// select which vertex buffer to display
	UINT stride = sizeof(TEXPOINT);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

	// select which primtive type we are using
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw the vertex buffer to the back buffer
	context->Draw(6, 0);

	srv->Release();
}
