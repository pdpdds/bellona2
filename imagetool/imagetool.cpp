// Multiboot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <string>
#include <iostream>
#include <Windows.h>
#include "ff.h"

FILE* g_rawDisk = 0;
FATFS g_fat32_system;

bool InsertFileToImage(int argc, char** argv);

int main(int argc, char** argv) {

	InsertFileToImage(argc, argv);

	if(g_rawDisk)
		fclose(g_rawDisk);
	
	return FALSE;
}


DWORD get_fattime(void)
{
	SYSTEMTIME tm;

	/* Get local time */
	GetLocalTime(&tm);

	/* Pack date and time into a DWORD variable */
	return   (tm.wYear - 1980) << 25 | tm.wMonth << 21 | tm.wDay << 16 | tm.wHour << 11 | tm.wMinute << 5 | tm.wSecond >> 1;
}


bool InsertFile(const char* srcName, const char* destName)
{
	FILE* source = fopen(srcName, "rb");

	if (source == 0)
		return false;


	FIL file;
	FRESULT res = f_open(&file, destName, FA_CREATE_ALWAYS | FA_WRITE);

	if (res != FR_OK)
	{
		printf("Image Target File Open Fail : %s %d\n", destName, res);
		return false;
	}

	char buffer[512];
	size_t bytes;

	UINT src_filesize = 0;
	UINT des_filesize = 0;

	while (0 < (bytes = fread(buffer, 1, 512 * sizeof(char), source)))
	{
		UINT written = 0;
		FRESULT result = f_write(&file, buffer, bytes, &written);

		src_filesize += bytes;
		des_filesize += written;


	}

	fclose(source);
	f_close(&file);

	if (src_filesize != des_filesize)
		return false;

	return true;
}

bool InsertFileToImage(int argc, char** argv)
{
	FRESULT result = (FRESULT)f_mount(&g_fat32_system, "", 1);

	if (result != FR_OK)
	{
		printf("fat32 initialization fail : %d\n", result);
		return false;
	}


	for (int i = 1; i < argc; i++)
	{
		if(false == InsertFile(argv[i], argv[i]))		
			return false;
	}
	
	return true;

}
#include <direct.h>
#define GetCurrentDir _getcwd


extern "C" BYTE IMG_disk_initialize()
{
	char buf[256];

	char* cwd = _getcwd(buf, 256);

	g_rawDisk = fopen("hdd.img", "rb+");

	if (g_rawDisk == 0)
		return 1;


	return 0;
}

extern "C" BYTE IMG_disk_read(BYTE * buff, DWORD sector, UINT count)
{
	fseek(g_rawDisk, sector * 512, SEEK_SET);
	size_t readCnt = fread(buff, 1, count * 512, g_rawDisk);

	if (readCnt != count * 512)
	{
		return 1;
	}

	return 0;
}

extern "C" BYTE IMG_disk_write(const BYTE * buff, DWORD sector, UINT count)
{
	fseek((FILE*)g_rawDisk, sector * 512, SEEK_SET);
	size_t writeCnt = fwrite(buff, 1, count * 512, (FILE*)g_rawDisk);

	if (writeCnt != count * 512)
	{
		return 1;
	}

	return 0;
}

extern "C" BYTE IMG_disk_status()
{
	return 0;
}