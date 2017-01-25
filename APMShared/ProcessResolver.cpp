#include "ProcessResolver.h"

HANDLE ProcessResolver::getProcessByName(const WCHAR* name) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 proc_entry = {0};
	proc_entry.dwSize = sizeof(PROCESSENTRY32);

	if(Process32First(hSnap, &proc_entry)) {
		do {
			if(_wcsicmp(name, proc_entry.szExeFile) == 0) {
				CloseHandle(hSnap);
				return OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, proc_entry.th32ProcessID);
			}
		} while(Process32Next(hSnap, &proc_entry));
	}

	CloseHandle(hSnap);
	return NULL;
}