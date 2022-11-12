#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

	void	EnablePaging(bool state);
	bool	IsPaging();

	//ĳ���� TLB�� ����.
	void InvalidateTLB(unsigned int va);

	//������ ���丮�� PDBR �������Ϳ� �����Ѵ�
	void SetPDBR(uint32_t dir);
	uint32_t GetPDBR();

	bool DetectionCPUID();

#ifdef __cplusplus
}
#endif

