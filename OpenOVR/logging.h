#pragma once

void oovr_log_raw(const char *file, long line, const char *func, const char *msg);
#define OOVR_LOG(msg) oovr_log_raw(__FILE__, __LINE__, __FUNCTION__, msg);
