#pragma once
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#ifdef __cplusplus
extern "C" {
#endif

	void* memcpy(void *dest, const void *src, size_t length);
	void* memset(void *start, int value, size_t size);
	int memcmp(const void *src1, const void *src2, size_t len);
	char *strchr(const char *str, int chr);
	unsigned long strlen(const char*);
	int strcmp(const char *str1, const char *str2);
	int strncmp(const char *str1, const char *str2, int size);
	char* strcpy(char *dest, const char *src);
	char* strcat(char *dest, const char *src);	
	char* strncpy(char *dest, const char *src, int len);
	const char* strrchr(const char* str, char const character);
	void itoa(unsigned i, unsigned base, char* buf);
	void itoa_s(unsigned int i, unsigned base, char* buf);
	char* _i64toa(long long value, char* str, int radix);
	void ftoa_fixed(char* buffer, double value);
	int strcmpi(const char* s1, const char* s2);


#ifdef __cplusplus
}
#endif
