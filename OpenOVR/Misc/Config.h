#pragma once

class Config {
public:
	Config();
	~Config();

	bool RenderCustomHands() const { return renderCustomHands; }
	vr::HmdColor_t HandColour() const { return handColour; }
	float SupersampleRatio() const { return supersampleRatio; }
	bool Haptics() const { return haptics; }
	float AdmitUnknownProps() const { return admitUnknownProps; }
	inline bool ThreePartSubmit() const { return threePartSubmit; }
	inline bool UseViewportStencil() const { return useViewportStencil; }
	inline bool ForceConnectedTouch() const { return forceConnectedTouch; }
	inline bool LogGetTrackedProperty() const { return logGetTrackedProperty; }
	inline bool EnableLayers() const { return enableLayers; }
	inline bool DX10Mode() const { return dx10Mode; }
	inline bool EnableAppRequestedCubemap() const { return enableAppRequestedCubemap; }

private:
	static int ini_handler(
		void* user, const char* section,
		const char* name, const char* value,
		int lineno);

	bool renderCustomHands = true;
	vr::HmdColor_t handColour = vr::HmdColor_t{ 0.3f, 0.3f, 0.3f, 1 };
	float supersampleRatio = 1.0f;
	bool haptics = true;
	bool admitUnknownProps = false;
	bool threePartSubmit = true;
	bool useViewportStencil = false;
	bool forceConnectedTouch = true;
	bool logGetTrackedProperty = false;

	// Default to false since this was preventing PAYDAY 2 from starting, need to investigate to find out
	//  if this is game-specific, or if it's a problem with the layer system
	bool enableLayers = true;

	bool dx10Mode = false;
	bool enableAppRequestedCubemap = true;
};

extern Config oovr_global_configuration;
