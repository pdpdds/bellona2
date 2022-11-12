#pragma once

#ifdef  __cplusplus
extern "C" {
#endif
	void* kmalloc(size_t size);
	void kfree(void* address);

#ifdef  __cplusplus
}
#endif