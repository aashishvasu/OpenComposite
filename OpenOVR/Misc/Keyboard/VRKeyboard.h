#pragma once
#include "OVR_CAPI.h"

#include <d3d11.h>

#include <vector>
#include <string>
#include <memory>
#include <codecvt>

#include "SudoFontMeta.h"
#include "KeyboardLayout.h"

class VRKeyboard {
public:
	VRKeyboard(ID3D11Device *dev, uint64_t userValue, uint32_t maxLength, bool minimal);
	~VRKeyboard();

	std::wstring contents();
	void contents(std::wstring);

	ovrLayerHeader * Update();

	void HandleOverlayInput(vr::EVREye controllerDeviceIndex, vr::VRControllerState_t state, float time);

	enum ECaseMode {
		LOWER,
		SHIFT,
		LOCK,
	};

	static std::wstring_convert<std::codecvt_utf8<wchar_t>> CHAR_CONV;

	bool IsClosed() { return closed; }

private:
	ID3D11Device * const dev;
	ID3D11DeviceContext *ctx;

	bool dirty = true;
	bool closed = false;

	std::wstring text;
	ECaseMode caseMode = LOWER;

	uint64_t userValue; // Arbitary user data, to be passed into the SteamVR events
	uint32_t maxLength;
	bool minimal;

	ovrTextureSwapChain chain;
	ovrTextureSwapChainDesc chainDesc;
	ovrLayerQuad layer;

	std::unique_ptr<SudoFontMeta> font;
	std::unique_ptr<KeyboardLayout> layout;

	// These use the OpenVR eye constants
	float lastInputTime[2];
	int repeatCount[2];
	int selected[2];
	uint64_t lastButtonState[2];

	void Refresh();
};
