// Multiboot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <string>
#include <iostream>
#include <Windows.h>
#include "ff.h"

FILE* g_rawDisk = 0;
FATFS g_fat32_system;

char szImageName[MAX_PATH] = "hdd.img";
bool InsertKernelToImageFile(int argc, char** argv);

using namespace std;
struct MULTIBOOT_HEADER {
	UINT32 magic;
	UINT32 flags;
	UINT32 checksum;
	UINT32 header_addr;
	UINT32 load_addr;
	UINT32 load_end_addr;
	UINT32 bss_end_addr;
	UINT32 entry_addr;
};

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}
void CreateProcessAndWait(char* proc) {
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));


	// Start the child process. 
	if (!CreateProcessA(NULL,   // No module name (use command line)
		proc,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
#define LOADBASE 0x00100000 // default load base of grub
int main(int argc, char** argv)
{

	//Open PE file.
	printf("Adding MultiBoot header to : %s.\n", argv[1]);
	HANDLE hFile = CreateFileA(argv[1], GENERIC_WRITE | GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("File not found.\n");
		return TRUE;
	}
	DWORD filesize = GetFileSize(hFile, 0);
	DWORD retsize;
	char* file = (char*)VirtualAlloc(0, filesize, MEM_COMMIT, PAGE_READWRITE);

	if (file == 0)
		return TRUE;

	if (ReadFile(hFile, file, filesize, &retsize, 0) == 0)
	{
		printf("Could not read the file Error :%X.\n", GetLastError());
		VirtualFree(file, 0, MEM_RELEASE);
		CloseHandle(hFile);
		return TRUE;
	}

	PIMAGE_DOS_HEADER DOS = (PIMAGE_DOS_HEADER)file;
	PIMAGE_NT_HEADERS NT = (PIMAGE_NT_HEADERS)(file + DOS->e_lfanew);
	if (NT->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("Not valid executable.\n");
		VirtualFree(file, 0, MEM_RELEASE);
		CloseHandle(hFile);
		return TRUE;
	}
	PIMAGE_SECTION_HEADER ISH = (PIMAGE_SECTION_HEADER)((char*)NT + sizeof(IMAGE_NT_HEADERS));
	if (((int)ISH - int(file) + NT->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER) + 0x20) > ISH[0].PointerToRawData)
	{
		printf("There is no enough space for multiboot header.\nNote:File Alignment should be 0x1000.\n");
		VirtualFree(file, 0, MEM_RELEASE);
		CloseHandle(hFile);
		return TRUE;
	}
	DWORD HeaderOffset = (DWORD)ISH - (DWORD)file + NT->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
	//Setup multiboot header.
	MULTIBOOT_HEADER header = {
		0x1BADB002,
		0x10003,
		-(0x1BADB002 + 0x10003),
		HeaderOffset + LOADBASE,
		LOADBASE,
		0,
		0,
		LOADBASE + NT->OptionalHeader.AddressOfEntryPoint
	};
	//Write the header to output file.
	memcpy(file + HeaderOffset, &header, sizeof(MULTIBOOT_HEADER));
	SetFilePointer(hFile, 0, 0, 0);
	if (WriteFile(hFile, file, filesize, &retsize, 0) == 0)
	{
		printf("Could not write to the file Error :%X.\n", GetLastError());
		VirtualFree(file, 0, MEM_RELEASE);
		CloseHandle(hFile);
		return TRUE;
	}
	CloseHandle(hFile);
	printf("Multiboot header has been added successfully.\n");


	if (InsertKernelToImageFile(argc, argv))
	{
		printf("kernel has been inserted to image file successfully.\n");
		fclose(g_rawDisk);
		// Use Qemu
		char command[0x200];
		//snprintf(command, sizeof(command), "\"qemu-system-i386.exe\" -kernel \"%s\"", argv[1]);
		snprintf(command, sizeof(command), "\"qemu-system-x86_64.exe\" -m 512 -hda %s -M pc -boot c", szImageName);

		CreateProcessAndWait((char*)command);
	}
	else
	{
		printf("kernel insert fail.\n");
	}

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

bool InsertKernelToImageFile(int argc, char** argv)
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

	if (false == InsertFile("grub.cfg", "0:/grub/grub.cfg"))
		return false;

	return true;

}
#include <direct.h>
#define GetCurrentDir _getcwd


extern "C" BYTE IMG_disk_initialize()
{
	char buf[256];

	char* cwd = _getcwd(buf, 256);

	g_rawDisk = fopen(szImageName, "rb+");

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