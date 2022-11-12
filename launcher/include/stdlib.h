#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t size);
void free(void*);
void srand(int new_seed);
int rand();

#ifdef __cplusplus
}
#endif
