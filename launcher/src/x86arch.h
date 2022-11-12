#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

	void	EnablePaging(bool state);
	bool	IsPaging();

	//캐쉬된 TLB를 비운다.
	void InvalidateTLB(unsigned int va);

	//페이지 디렉토리를 PDBR 레지스터에 설정한다
	void SetPDBR(uint32_t dir);
	uint32_t GetPDBR();

	bool DetectionCPUID();

#ifdef __cplusplus
}
#endif

