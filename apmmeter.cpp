#include "apmmeter.h"
#include <stdio.h>
#include <tchar.h>
#include <QDebug>

APMMeter::APMMeter()
{
    measure = new APMMeasure();
    stopped = true;
    initialized = false;
}

APMMeter::~APMMeter()
{
    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);
    delete measure;
}


int APMMeter::init() {

    hinstAPMSharedDll = LoadLibrary(TEXT("APMKeyHook.dll"));
    hprocKeyboard = (HOOKPROC)GetProcAddress(hinstAPMSharedDll, "_ZN10APMKeyHook12KeyboardProcEijl@12");
    hprocMouse = (HOOKPROC)GetProcAddress(hinstAPMSharedDll, "_ZN10APMKeyHook9MouseProcEijl@12");
    if(hinstAPMSharedDll == NULL || hprocKeyboard == NULL || hprocMouse == NULL) {
        qDebug("Failed to load APMKeyHook.dll");
        if(hinstAPMSharedDll == NULL)
            qDebug() << "DLL not found" << GetLastError();
        else
            qDebug() << "Couldn't find procedure (incompatible dll)" << GetLastError();
        return 1;
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD, hprocKeyboard, hinstAPMSharedDll, 0);
    mouseHook = SetWindowsHookEx(WH_MOUSE, hprocMouse, hinstAPMSharedDll, 0);
    if(keyboardHook == NULL || mouseHook == NULL) {
        qDebug() << "Failed to register hooks for APM meter" << GetLastError();
        return 2;
    }

    return 0;
}

void APMMeter::start()
{
    if(!initialized)
    {
        if(init()!=0)
            return;
        else
            initialized = true;
    }

    stopped = false;

    // нужно обязатель сбросить все счетчики, иначе время будет считаться неверно
    measure->resetAllAPM();

    timerId = SetTimer(NULL, 0, MEASURE_CYCLE_LENGTH-10, NULL);
    qDebug() << "APMMeter started";
    max = 0;
    calc_max = false;
    MSG msg = MSG();
    while(!stopped)
    {
        // получаем сообщение
        BOOL bRet = GetMessage(&msg, NULL, 0, 0);
        if(bRet == 0 || bRet == -1)
            break;

        // если это сообщение таймера
        if(msg.message == WM_TIMER) {
            if(calc_max){
                // получаем текущий APM (текущий APM это APM вычисляемый из 20 последних действий)
                long current = measure->getAverageAPM();
                // если текущий APM больше максимального, то обновляем максимальный
                if(current > max)
                    max = current;
            }
            // выводим текущий APM, средний и максимальный
            measure->moveCurrentAPM();
        }
        DispatchMessage(&msg);
    }

    qDebug() << "APMMeter stopped";
    KillTimer(NULL, timerId);
}

void APMMeter::stop()
{
    stopped = true;
}

long APMMeter::getTotalActions()
{
    return measure->getTotalActions();
}

DWORD APMMeter::getTime()
{
    return measure->getTime();
}

long APMMeter::getAverageAPM()
{
    return measure->getAverageAPM();
}

long APMMeter::getCurrentAPM()
{
    return measure->getCurrentAPM();
}

long APMMeter::getMaxAPM()
{
    calc_max = true;

    if(!calc_max)
        return measure->getAverageAPM();

    return max;
}

