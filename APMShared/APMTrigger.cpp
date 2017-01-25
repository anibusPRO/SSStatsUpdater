#include "APMTrigger.h"
#include <stdio.h>
#include <string.h>
#include <QDebug>

// конструктор класса APMTrigger
APMTrigger::APMTrigger(APMConfig* n_cfg) {
	cfg = n_cfg;
	hProcess = NULL;

	switch(cfg->trigger_method) {
    // если запуск по хоткею
	case TRIGGER_BY_HOTKEY:
        // то регистрируем хоткей


//        BOOL RegisterHotKey
//        (
//            HWND hWnd,		// окно, которое принимает уведомление "горячей" клавиши
//            int id,		// идентификатор "горячей" клавиши
//            UINT fsModifiers,	// флажки модификации клавиш
//            UINT vk		// код виртуальной клавиши
//        );

        if(!RegisterHotKey(NULL, HOTKEY_STARTSTOP_MEASURE, MOD_ALT | MOD_SHIFT, 0x50)) {
			exit(GetLastError());
		}
		break;
	case TRIGGER_BY_PROCESS:
		break;
	}
}

APMTrigger::~APMTrigger() {
	switch(cfg->trigger_method) {
	case TRIGGER_BY_HOTKEY:
		UnregisterHotKey(NULL, HOTKEY_STARTSTOP_MEASURE);
		break;
	case TRIGGER_BY_PROCESS:
		CloseHandle(hProcess);
		break;
	}
}

BOOL APMTrigger::triggerStart() {
	switch(cfg->trigger_method) {
	case TRIGGER_BY_HOTKEY:
		return triggerStartByHotkey();
	case TRIGGER_BY_PROCESS:
		return triggerStartByProcess();
	default:
		return TRUE;
	}
}

BOOL APMTrigger::triggerStartByHotkey() {
	MSG msg = {0};
	while(true) {

//        BOOL GetMessage
//        (
//            LPMSG lpMsg, // указатель на структуру
//            HWND hWnd, // указатель окна чьи сообщения нужно обрабатывать
//            UINT wMsgFilterMin, // номер мимимального сообщения для выборки
//            UINT wMsgFilterMax // номер максимального сообщения для выборки
//        );

		BOOL bRet = GetMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY);

		if(bRet == 0 || bRet == -1)
            exit(bRet);

		if(msg.wParam == HOTKEY_STARTSTOP_MEASURE) {
			return TRUE;
		}
	}
}

BOOL APMTrigger::triggerStartByProcess() {
	while(!(hProcess = getProcessByName(cfg->trigger_process)))
		Sleep(50);
	return TRUE;
}

BOOL APMTrigger::triggerStop(MSG* msg) {
	switch(cfg->trigger_method) {
	case TRIGGER_BY_HOTKEY:
		return triggerStopByHotkey(msg);
	case TRIGGER_BY_PROCESS:
		return triggerStopByProcess();
	default:
		return TRUE;
	}
}

BOOL APMTrigger::triggerStopByHotkey(MSG* msg) {
	if(msg->message == WM_HOTKEY && msg->wParam == HOTKEY_STARTSTOP_MEASURE)
		return TRUE;
	return FALSE;
}

BOOL APMTrigger::triggerStopByProcess() {
	if(WaitForSingleObject(hProcess, 0) == WAIT_OBJECT_0)
		return TRUE;
	return FALSE;
}

HANDLE APMTrigger::getProcessByName(const char* name) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 proc_entry = {0};
	proc_entry.dwSize = sizeof(PROCESSENTRY32);

    wchar_t wtext[20];
    mbstowcs(wtext, name, strlen(name)+1);

	if(Process32First(hSnap, &proc_entry)) {
		do {
            if(_wcsicmp(wtext, proc_entry.szExeFile) == 0) {
				CloseHandle(hSnap);
				return OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, proc_entry.th32ProcessID);
			}
		} while(Process32Next(hSnap, &proc_entry));
	}

	CloseHandle(hSnap);
	return NULL;
}
