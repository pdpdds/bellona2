#pragma once
#include <stdint.h>

#define LOG_DEBUG	0
#define LOG_INFO	1
#define LOG_WARN	2
#define LOG_FATAL	3

#ifdef  __cplusplus
extern "C" {
#endif

	void LOG(uint8_t loglevel, char* fmt, ...);
	void SetLogLevel(uint8_t loglevel);
	void InitLogManager(uint8_t loglevel);

#ifdef  __cplusplus
}
#endif