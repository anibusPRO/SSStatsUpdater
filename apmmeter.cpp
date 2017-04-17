#include "apmmeter.h"
#include "APMShared\ProcessResolver.h"
#include "APMShared\APMConfig.h"
#include <QDebug>
#include <stdio.h>
#include <tchar.h>


APMMeter::APMMeter()
{
    logger = NULL;
    cfg = new APMConfig();
    measure = new APMMeasure(cfg);
    stop_flag = true;
}

APMMeter::~APMMeter()
{
    delete measure;
    if(logger!=NULL) delete logger;
    delete cfg;
}


int APMMeter::init() {

    // если указано вести запись в лог файл, то создадим логгер (по умолчанию выключено)
    if(cfg->log_file)
        logger = new APMLogger(*cfg->log_file);

    return 0;
}

void APMMeter::mainCycle()
{
    hinstAPMSharedDll = LoadLibrary(TEXT("APMKeyHook.dll"));
    hprocKeyboard = (HOOKPROC)GetProcAddress(hinstAPMSharedDll, "_ZN10APMKeyHook12KeyboardProcEijl@12");
    hprocMouse = (HOOKPROC)GetProcAddress(hinstAPMSharedDll, "_ZN10APMKeyHook9MouseProcEijl@12");
    if(hinstAPMSharedDll == NULL || hprocKeyboard == NULL || hprocMouse == NULL) {
        qDebug("Failed to load APMKeyHook.dll");
        if(hinstAPMSharedDll == NULL)
            qDebug("DLL not found");
        else
            qDebug("Couldn't find procedure (incompatible dll)");
        return /*GetLastError()*/;
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD, hprocKeyboard, hinstAPMSharedDll, 0);
    mouseHook = SetWindowsHookEx(WH_MOUSE, hprocMouse, hinstAPMSharedDll, 0);
    if(keyboardHook == NULL || mouseHook == NULL) {
        qDebug("Failed to register hooks for APM meter");
        return /*GetLastError()*/;
    }

    timerId = SetTimer(NULL, 0, MEASURE_CYCLE_LENGTH-10, NULL);
    qDebug() << "meter started";
    max = 0;
    calc_max = false;
    MSG msg = {0};
    while(!stop_flag)
    {
        // получаем сообщение
        BOOL bRet = GetMessage(&msg, NULL, 0, 0);
        if(bRet == 0 || bRet == -1)
            break;

        // если это сообщение таймера
        if(msg.message == WM_TIMER) {
            if(calc_max){
                // получаем текущий APM (текущий APM это APM вычисляемый из 20 последних действий)
                long current = measure->getCurrentAPM();
                // если текущий APM больше максимального, то обновляем максимальный
                if(current > max)
                    max = current;
            }
            // выводим текущий APM, средний и максимальный
            measure->moveCurrentAPM();

            // если включено логирование, то делаем запись в лог
            if(logger != NULL)
                logger->addEntry(measure->getSnapshot());
        }
        DispatchMessage(&msg);
    }
    qDebug() << "meter stopped";
    KillTimer(NULL, timerId);

    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);
}

void APMMeter::start()
{
    stop_flag = false;
    // нужно обязатель сбросить все счетчики, иначе время будет считаться неверно
    measure->resetAllAPM();
    mainCycle();
}

void APMMeter::stop()
{
    stop_flag = true;
}

long APMMeter::getTotalActions()
{
    return measure->getTotalActions();
}

long APMMeter::getTime()
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
    if(!calc_max)
        return measure->getCurrentAPM();
    calc_max = true;
    return max;
}
