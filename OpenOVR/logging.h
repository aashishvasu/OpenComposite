#pragma once

void oovr_log_raw(const char *file, long line, const char *func, const char *msg);
void oovr_log_raw_format(const char *file, long line, const char *func, const char *msg, ...);
#define OOVR_LOG(msg) oovr_log_raw(__FILE__, __LINE__, __FUNCTION__, msg);
#define OOVR_LOGF(...) oovr_log_raw_format(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__);

void oovr_abort_raw(const char *file, long line, const char *func, const char *msg, const char *title = nullptr);
#define OOVR_ABORT(msg) { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg); throw msg; }
#define OOVR_ABORT_T(msg, title) { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg, title); throw msg; }

// DirectX API validation helpers
#define OOVR_FAILED_DX_ABORT(expression) \
{ \
HRESULT res = (expression); \
if (FAILED(res)) { \
	OOVR_LOGF("DX Call failed with: 0x%08x", res); \
	OOVR_ABORT_T("OOVR_FAILED_DX_ABORT failed on: " #expression, "OpenComposite DirectX error - see log for details") \
} \
}

// General validation helpers
#define OOVR_FALSE_ABORT(expression) if (!(expression)) { OOVR_ABORT("Expression is false unexpectedly: " #expression); }

// OVR API validation helpers
#define OOVR_FAILED_OVR_LOG(expression) \
if (!OVR_SUCCESS(expression)) { \
	ovrErrorInfo e = {}; \
	ovr_GetLastErrorInfo(&e); \
	OOVR_LOGF("OVR Call failed.  Error code: %d  Descr: %s", e.Result, e.ErrorString); \
}

#define OOVR_FAILED_OVR_ABORT(expression) \
if (!OVR_SUCCESS(expression)) { \
	ovrErrorInfo e = {}; \
	ovr_GetLastErrorInfo(&e); \
	std::string err = "OVR Call failed, aborting.  Error code: " + std::to_string(e.Result) + "  Descr: " + std::string(e.ErrorString); \
	OOVR_ABORT(err.c_str()); \
}

// Yay for there not being a PI constant in the standard
// (technically it has absolutely nothing to do with logging but this is a convenient place to put it)
const extern float math_pi;
