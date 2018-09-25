#include "stdafx.h"
#include "Config.h"
#include "ini.h"

#include <string>
#include <algorithm>

using namespace std;

Config oovr_global_configuration;

// OOVR_ABORT doesn't work here for some reason
#define ABORT(msg) { MessageBoxA(NULL, string(msg).c_str(), "OpenOVR Config File Error", MB_OK); exit(1); }

static string str_tolower(std::string val) {
	transform(val.begin(), val.end(), val.begin(), ::tolower);
	return val;
}

static bool parse_bool(string orig, string name, int line) {
	string val = str_tolower(orig);

	if (val == "true" || val == "on" || val == "enabled")
		return true;
	if (val == "false" || val == "off" || val == "disabled")
		return false;

	string err = "Unknown value " + orig + " for in config file for " + name + " on line " + to_string(line);
	ABORT(err);
}

int Config::ini_handler(void* user, const char* pSection,
	const char* pName, const char* pValue,
	int lineno) {

	string section = pSection;
	string name = pName;
	string value = pValue;

	Config *cfg = (Config*)user;

#define CFGOPT(type, vname) if(name == #vname) cfg->vname = parse_ ## type(value, #vname, lineno)

	if (section == "" || section == "default") {
		CFGOPT(bool, enableAudio);
	}

#undef CFGOPT

	// Success
	return true;
}

static int wini_parse(const wchar_t* filename, ini_handler handler, void* user) {
	FILE* file;
	errno_t err = _wfopen_s(&file, filename, L"r");
	if (err)
		return -1;

	int error = ini_parse_file(file, handler, user);
	fclose(file);
	return error;
}

// The ctor is run before DLLMain, so use this hack for now
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

Config::Config() {
	wchar_t buffer[MAX_PATH];
	DWORD len = GetModuleFileNameW(HINST_THISCOMPONENT, buffer, sizeof(buffer));

	wstring dir = L"";
	if (len) {
		wstring fname = wstring(buffer, len);
		size_t slash_index = fname.rfind(L'\\');

		if (slash_index != wstring::npos) {
			dir = fname.substr(0, slash_index + 1);
		}
	}

	wstring file = dir + L"opencomposite.ini";
	int err = wini_parse(file.c_str(), ini_handler, this);

	if (err == -1) {
		// Couldn't open file, no problen since the config file is optional and
		//  the defaults are set up as the default values for the variables
		return;
	}
	else if (err) {
		// err is the line number
		string str = "Config error on line " + to_string(err);
		ABORT(str);
	}

	// Everything should have been set up by ini_handler
}

Config::~Config() {
}
