#include "stdafx.h"

#include "Config.h"
#include "ini.h"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>

#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using vr::HmdColor_t;

Config oovr_global_configuration;

// OOVR_ABORT doesn't work here for some reason
// TODO Turtle1331 use OOVR_ABORT from logging.h
#ifdef WIN32
#define ABORT(msg)                                                                        \
	{                                                                                     \
		MessageBoxA(NULL, string(msg).c_str(), "OpenComposite Config File Error", MB_OK); \
		exit(1);                                                                          \
	}
#else
#define ABORT(msg) \
	{              \
		exit(42);  \
	}
#endif

static string str_tolower(std::string val)
{
	transform(val.begin(), val.end(), val.begin(), ::tolower);
	return val;
}

unsigned char hexval(unsigned char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	else if ('a' <= c && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

static bool parse_bool(string orig, string name, int line)
{
	string val = str_tolower(orig);

	if (val == "true" || val == "on" || val == "enabled") {
		OOVR_LOGF("Setting config param %s to true", name.c_str());
		return true;
	}
	if (val == "false" || val == "off" || val == "disabled") {
		OOVR_LOGF("Setting config param %s to false", name.c_str());
		return false;
	}

	string err = "Value " + orig + " for in config file for " + name + " on line "
	    + to_string(line) + " is not a boolean - true/on/enabled/false/off/disabled";
	ABORT(err);
}

static HmdColor_t parse_HmdColor_t(string orig, string name, int line)
{
	string val = str_tolower(orig);

	if (val.length() != 7 || val[0] != '#') {
		goto invalid;
	}

	for (int i = 1; i < val.length(); i++) {
		char c = val[i];
		if ((c < '0' || c > '9') && (c < 'a' || c > 'f'))
			goto invalid;
	}

	HmdColor_t c;
	c.a = 255; // Full alpha

	c.r = ((hexval(val[1]) << 4) + hexval(val[2])) / 255.0f;
	c.g = ((hexval(val[3]) << 4) + hexval(val[4])) / 255.0f;
	c.b = ((hexval(val[5]) << 4) + hexval(val[6])) / 255.0f;

	OOVR_LOGF("Setting config param %s to %f %f %f", name.c_str(), c.r, c.g, c.b);
	return c;

invalid:

	string err = "Value " + orig + " for in config file for " + name + " on line "
	    + to_string(line) + " is not a hex (CSS) colour code";
	ABORT(err);
}

static float parse_float(string orig, string name, int line)
{
	string val = str_tolower(orig);

	const char* str = orig.c_str();
	char* end = NULL;
	float result = strtof(str, &end);

	if (end != str + orig.length()) {
		string err = "Value " + orig + " for in config file for " + name + " on line "
		    + to_string(line) + " is not a decimal number (eg 12.34)";
		ABORT(err);
	}

	OOVR_LOGF("Setting config param %s to %f", name.c_str(), result);
	return result;
}

int Config::ini_handler(void* user, const char* pSection,
    const char* pName, const char* pValue,
    int lineno)
{

	string section = pSection;
	string name = pName;
	string value = pValue;

	Config* cfg = (Config*)user;

#define CFGOPT(type, vname)                             \
	if (name == #vname) {                               \
		cfg->vname = parse_##type(value, name, lineno); \
		return true;                                    \
	}

	if (section == "" || section == "default") {
		CFGOPT(bool, renderCustomHands);
		CFGOPT(HmdColor_t, handColour);
		CFGOPT(float, supersampleRatio);
		CFGOPT(bool, haptics);
		CFGOPT(bool, admitUnknownProps);
		CFGOPT(bool, threePartSubmit);
		CFGOPT(bool, useViewportStencil);
		CFGOPT(bool, forceConnectedTouch);
		CFGOPT(bool, logGetTrackedProperty);
		CFGOPT(bool, stopOnSoftAbort);
		CFGOPT(bool, enableLayers);
		CFGOPT(bool, dx10Mode);
		CFGOPT(bool, enableAppRequestedCubemap);
		CFGOPT(bool, enableHiddenMeshFix);
		CFGOPT(bool, invertUsingShaders);
		CFGOPT(bool, initUsingVulkan);
		CFGOPT(float, hiddenMeshVerticalScale);
	}

#undef CFGOPT

	string err = "Unknown config option " + name + " on line " + to_string(lineno);
	ABORT(err);
}

static int wini_parse(const wchar_t* filename, ini_handler handler, void* user)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> CHAR_CONV;
	std::string utf8filename = CHAR_CONV.to_bytes(filename);

	FILE* file = fopen(utf8filename.c_str(), "r");
	if (!file) {
		OOVR_LOGF("No config file found at %s", utf8filename.c_str());
		return -1;
	}

	OOVR_LOGF("Reading config file at %s", utf8filename.c_str());

	int error = ini_parse_file(file, handler, user);
	fclose(file);
	return error;
}

// The ctor is run before DLLMain, so use this hack for now
#ifdef _WIN32
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

Config::Config()
{
	// If we're on Windows, look for a config file next to the DLL
	// If we're on Linux, skip that and just check the working directory.
#ifdef _WIN32
	wchar_t buffer[MAX_PATH];
	DWORD len = GetModuleFileNameW(HINST_THISCOMPONENT, buffer, sizeof(buffer));

	wstring dir;
	if (len) {
		wstring fname = wstring(buffer, len);
		size_t slash_index = fname.rfind(L'\\');

		if (slash_index != wstring::npos) {
			dir = fname.substr(0, slash_index + 1);
		}
	}

	OOVR_LOG("Checking for global config file...");
	wstring file = dir + L"opencomposite.ini";
	int err = wini_parse(file.c_str(), ini_handler, this);
#else
	int err = -1;
	wstring file;
#endif

	if (err == -1 || err == 0) {
		// No such file or it was parsed successfully, check the working directory
		// for a file that overrides some properties
		char buff[FILENAME_MAX];
		GetCurrentDir(buff, FILENAME_MAX);
		file = wstring(&buff[0], &buff[strlen(buff)]);
#ifdef _WIN32
		file += L"\\opencomposite.ini";
#else
		file += L"/opencomposite.ini";
#endif
		OOVR_LOG("Checking for app specific config file...");
		err = wini_parse(file.c_str(), ini_handler, this);
	}

	if (err == -1) {
		// Couldn't open file, no problen since the config file is optional and
		//  the defaults are set up as the default values for the variables
		return;
	} else if (err) {
		// err is the line number
		string str = "Config error on line " + to_string(err);
		ABORT(str);
	}

	// Everything should have been set up by ini_handler
}

Config::~Config()
{
}
