#pragma once

void oovr_log_raw(const char *file, long line, const char *func, const char *msg);
#define OOVR_LOG(msg) oovr_log_raw(__FILE__, __LINE__, __FUNCTION__, msg);

void oovr_abort_raw(const char *file, long line, const char *func, const char *msg);
#define OOVR_ABORT(msg) { oovr_abort_raw(__FILE__, __LINE__, __FUNCTION__, msg); throw msg; }

// Yay for there not being a PI constant in the standard
// (technically it has absolutely nothing to do with logging but this is a convenient place to put it)
const extern float math_pi;
