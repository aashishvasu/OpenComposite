#pragma once

class Config {
public:
	Config();
	~Config();

	bool EnableAudio() const { return enableAudio; }

private:
	static int ini_handler(
		void* user, const char* section,
		const char* name, const char* value,
		int lineno);

	bool enableAudio = true;
};

extern Config oovr_global_configuration;
