#include "proc.h"
#include <cstdlib>
#include <iostream>

DWORD GetProcId(const wchar_t* procName) {

	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE) {

		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);
		if (Process32First(hSnap, &procEntry)){
			do {
				if (!_wcsicmp(procEntry.szExeFile, procName)) {
					procId = procEntry.th32ProcessID;
				} 
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}


uintptr_t GetBaseAddress(DWORD procId) {

	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(modEntry);

	if (Module32First(hSnap, &modEntry))
	{
		CloseHandle(hSnap);
		return (DWORD_PTR)modEntry.modBaseAddr;
	}

	CloseHandle(hSnap);
	return 1;
}


