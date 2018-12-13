#pragma once
#include "OVR_CAPI.h"

#include <d3d11.h>

#include <vector>
#include <string>
#include <memory>

#include "SudoFontMeta.h"
#include "KeyboardLayout.h"

class VRKeyboard {
public:
	VRKeyboard(ID3D11Device *dev);
	~VRKeyboard();

	std::wstring contents();
	void contents(std::wstring);

	ovrLayerHeader * Update();

	enum ECaseMode {
		LOWER,
		SHIFT,
		LOCK,
	};

private:
	ID3D11Device * const dev;
	ID3D11DeviceContext *ctx;

	bool dirty = true;

	std::wstring text;
	ECaseMode caseMode = LOWER;

	ovrTextureSwapChain chain;
	ovrTextureSwapChainDesc chainDesc;
	ovrLayerQuad layer;

	std::unique_ptr<SudoFontMeta> font;
	std::unique_ptr<KeyboardLayout> layout;

	void Refresh();
};

