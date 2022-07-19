#pragma once

// Not strictly a logging thing, but makes clion happy about calling OOVR_ABORT and not returning
#ifdef _WIN32
#define OC_NORETURN __declspec(noreturn)
#else
#define OC_NORETURN __attribute__((noreturn))
#endif

void oovr_log_raw(const char* file, long line, const char* func, const char* msg);
void oovr_log_raw_format(const char* file, long line, const char* func, const char* msg, ...);
#define OOVR_LOG(msg) oovr_log_raw(__FILE__, __LINE__, __FUNCTION__, msg)
#define OOVR_LOGF(...) oovr_log_raw_format(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

OC_NORETURN void oovr_abort_raw(const char* file, long line, const char* func, const char* msg, const char* title = nullptr, ...);
void oovr_soft_abort_raw(const char* file, long line, const char* func, int* hit_count, const char* msg, ...);

#define OOVR_ABORT(msg)                                        \
	do {                                                       \
		oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg); \
	} while (0)
#define OOVR_ABORT_T(msg, title)                                      \
	do {                                                              \
		oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg, title); \
	} while (0)
#define OOVR_ABORTF(msg, ...)                                                        \
	do {                                                                             \
		oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg, nullptr, __VA_ARGS__); \
	} while (0)

/**
 * Perform a 'soft abort'. If OpenComposite is in debug mode this will cause a normal (hard) abort, but
 * otherwise will log a message once into the log and continue.
 *
 * This is designed to be used in a place where the game has requested some action that's not yet supported
 * and OpenComposite wants to mark it for developer attention while investigating the game, but at the same
 * time the game will probably continue working in a degraded state without this (for example, missing
 * haptics).
 *
 * Previously hard aborts were used for this, and unsurprisingly it caused a lot of trouble where what
 * should have been minor issues were making games crash at startup.
 */
#define OOVR_SOFT_ABORTF(msg, ...)                                                                      \
	do {                                                                                                \
		static int soft_abort_hit_count = 0;                                                            \
		oovr_soft_abort_raw(__FILE__, __LINE__, __FUNCTION__, &soft_abort_hit_count, msg, __VA_ARGS__); \
	} while (0)

// Work on GCC since it needs some varargs, and the actual value doesn't matter
#define OOVR_SOFT_ABORT(msg) OOVR_SOFT_ABORTF(msg, nullptr)

// Log once function - useful for warning that a function called many times isn't implemented, while using
// a workaround to make something mostly work.
#define OOVR_LOG_ONCE(msg)                                                                         \
	do {                                                                                           \
		/* We don't need the function suffix, but it makes it easier to recognise in a debugger */ \
		static bool oovr_hit_log_once_##__FUNCTION__ = false;                                      \
		if (!oovr_hit_log_once_##__FUNCTION__) {                                                   \
			oovr_hit_log_once_##__FUNCTION__ = true;                                               \
			OOVR_LOG("[once] " msg);                                                               \
		}                                                                                          \
	} while (false)

void oovr_message_raw(const char* message, const char* title);
#define OOVR_MESSAGE(message, title) oovr_message_raw(message, title);

// DirectX API validation helpers
#define OOVR_FAILED_DX_ABORT(expression)                                                                                       \
	do {                                                                                                                       \
		HRESULT res = (expression);                                                                                            \
		if (FAILED(res)) {                                                                                                     \
			OOVR_LOGF("DX Call failed with: 0x%08x", res);                                                                     \
			OOVR_ABORT_T("OOVR_FAILED_DX_ABORT failed on: " #expression, "OpenComposite DirectX error - see log for details"); \
		}                                                                                                                      \
	} while (0)

#define OOVR_FAILED_VK_ABORT(expression)                                              \
	do {                                                                              \
		VkResult failed_vk_abort_result = (expression);                               \
		if (failed_vk_abort_result < VK_SUCCESS) {                                    \
			OOVR_ABORTF("Vulkan Call failed, aborting. %s:%d %s. Error code: %d\n%s", \
			    __FILE__, __LINE__, __func__, failed_vk_abort_result, #expression);   \
		}                                                                             \
	} while (0)

// General validation helpers
#define OOVR_FALSE_ABORT(expression)                                      \
	do {                                                                  \
		if (!(expression)) {                                              \
			OOVR_ABORT("Expression is false unexpectedly: " #expression); \
		}                                                                 \
	} while (0)

// OVR API validation helpers
#define OOVR_FAILED_OVR_LOG(expression)                                                    \
	if (!OVR_SUCCESS(expression)) {                                                        \
		ovrErrorInfo e = {};                                                               \
		ovr_GetLastErrorInfo(&e);                                                          \
		OOVR_LOGF("OVR Call failed.  Error code: %d  Descr: %s", e.Result, e.ErrorString); \
	}

// TODO does OpenXR have an equivalent of ovr_GetLastErrorInfo?
#define OOVR_FAILED_XR_ABORT(expression)                                              \
	do {                                                                              \
		XrResult failed_xr_abort_result = (expression);                               \
		if (XR_FAILED(failed_xr_abort_result)) {                                      \
			OOVR_ABORTF("OpenXR Call failed, aborting. %s:%d %s. Error code: %d\n%s", \
			    __FILE__, __LINE__, __func__, failed_xr_abort_result, #expression);   \
		}                                                                             \
	} while (0)

#define OOVR_FAILED_XR_SOFT_ABORT(expression)                                       \
	do {                                                                            \
		XrResult failed_xr_abort_result = (expression);                             \
		if (XR_FAILED(failed_xr_abort_result)) {                                    \
			OOVR_SOFT_ABORTF("OpenXR Call failed. %s:%d %s. Error code: %d\n%s",    \
			    __FILE__, __LINE__, __func__, failed_xr_abort_result, #expression); \
		}                                                                           \
	} while (0)

// Yay for there not being a PI constant in the standard
// (technically it has absolutely nothing to do with logging but this is a convenient place to put it)
constexpr float math_pi = 3.14159265358979323846f;

// Again unrelated and just putting it here because everything can use it. This makes using
// strcpy_s more convenient on arrays in a cross-platform way.
#define strcpy_arr(dest, src) strcpy_s(dest, sizeof(dest), src)
