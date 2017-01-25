#pragma once
#include "APMConfig.h"
#include <windows.h>
#include <TlHelp32.h>

class APMTrigger {
private:
	APMConfig* cfg;
	HANDLE hProcess;

    static HANDLE getProcessByName(const char* name);

	BOOL triggerStartByHotkey();
	BOOL triggerStartByProcess();
	BOOL triggerStopByHotkey(MSG* msg);
	BOOL triggerStopByProcess();
public:
	APMTrigger(APMConfig* n_cfg);
	~APMTrigger();
	BOOL triggerStart();
	BOOL triggerStop(MSG* msg);
};
