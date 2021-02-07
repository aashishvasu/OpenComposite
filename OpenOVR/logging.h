#pragma once

void oovr_log_raw(const char *file, long line, const char *func, const char *msg);
void oovr_log_raw_format(const char *file, long line, const char *func, const char *msg, ...);
#define OOVR_LOG(msg) oovr_log_raw(__FILE__, __LINE__, __FUNCTION__, msg)
#define OOVR_LOGF(...) oovr_log_raw_format(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

void oovr_abort_raw(const char *file, long line, const char *func, const char *msg, const char *title = nullptr, ...);
#define OOVR_ABORT(msg) do { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg); throw msg; } while(0)
#define OOVR_ABORT_T(msg, title) do { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg, title); throw msg; } while(0)
#define OOVR_ABORTF(msg, ...) do { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg, nullptr, __VA_ARGS__); throw msg; } while(0)

// DirectX API validation helpers
#define OOVR_FAILED_DX_ABORT(expression) \
do { \
HRESULT res = (expression); \
if (FAILED(res)) { \
	OOVR_LOGF("DX Call failed with: 0x%08x", res); \
	OOVR_ABORT_T("OOVR_FAILED_DX_ABORT failed on: " #expression, "OpenComposite DirectX error - see log for details"); \
} \
} while(0)

// General validation helpers
#define OOVR_FALSE_ABORT(expression) do { if (!(expression)) { OOVR_ABORT("Expression is false unexpectedly: " #expression); } } while(0)

// OVR API validation helpers
#define OOVR_FAILED_OVR_LOG(expression) \
if (!OVR_SUCCESS(expression)) { \
	ovrErrorInfo e = {}; \
	ovr_GetLastErrorInfo(&e); \
	OOVR_LOGF("OVR Call failed.  Error code: %d  Descr: %s", e.Result, e.ErrorString); \
}

// TODO does OpenXR have an equivalent of ovr_GetLastErrorInfo?
#define OOVR_FAILED_XR_ABORT(expression)                                                \
	do {                                                                                \
		XrResult failed_xr_abort_result = (expression);                                 \
		if (XR_FAILED(failed_xr_abort_result)) {                                        \
			OOVR_ABORTF("OpenXR Call failed, aborting. %s:%d %s. Error code: %d\n%s", \
			    __FILE__, __LINE__, __func__, failed_xr_abort_result, #expression);     \
		}                                                                               \
	} while(0)

// Yay for there not being a PI constant in the standard
// (technically it has absolutely nothing to do with logging but this is a convenient place to put it)
const extern float math_pi;
