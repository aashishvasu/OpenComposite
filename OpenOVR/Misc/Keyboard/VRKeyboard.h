#pragma once
#include "OVR_CAPI.h"

#include <d3d11.h>

#include <vector>
#include <string>
#include <memory>
#include <codecvt>
#include <functional>

#include "SudoFontMeta.h"
#include "KeyboardLayout.h"

class VRKeyboard {
public:
	typedef std::function<void(vr::VREvent_t)> eventDispatch_t;

	// Yay this isn't in vrtypes.h
	// Maybe we should make the header splitter put enums somewhere else?
	enum EGamepadTextInputMode {
		k_EGamepadTextInputModeNormal = 0,
		k_EGamepadTextInputModePassword = 1,
		k_EGamepadTextInputModeSubmit = 2,
	};

	VRKeyboard(ID3D11Device *dev, uint64_t userValue, uint32_t maxLength, bool minimal, eventDispatch_t dispatch, EGamepadTextInputMode inputMode);
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

	void SetTransform(vr::HmdMatrix34_t transform);

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
	eventDispatch_t eventDispatch;
	EGamepadTextInputMode inputMode;

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

	void SubmitEvent(vr::EVREventType ev, wchar_t ch);
};
