#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>

class ProcessResolver {
public:
	static HANDLE getProcessByName(const WCHAR* name);
};