#include "apmmeter.h"
//#include "APMKeyHook\targetver.h"
#include "APMShared\ProcessResolver.h"
#include "APMShared\APMConfig.h"
#include <QDebug>
#include <stdio.h>
#include <tchar.h>


APMMeter::APMMeter()
{
    logger = NULL;
    cfg = new APMConfig(/*argc, argv*/);
//    trigger = new APMTrigger(cfg);
    measure = new APMMeasure(cfg);
    stop_flag = true;
}

APMMeter::~APMMeter()
{
    // убиваем таймер по его id
//    KillTimer(NULL, timerId);


    delete measure;
    if(logger!=NULL) delete logger;
//    delete trigger;
    delete cfg;
}


int APMMeter::init(/*int argc, char* argv[]*/) {


//    hinstAPMSharedDll = LoadLibrary(TEXT("APMKeyHook.dll"));
//    hprocKeyboard = (HOOKPROC)GetProcAddress(hinstAPMSharedDll, "_ZN10APMKeyHook12KeyboardProcEijl@12");
//    hprocMouse = (HOOKPROC)GetProcAddress(hinstAPMSharedDll, "_ZN10APMKeyHook9MouseProcEijl@12");
//    if(hinstAPMSharedDll == NULL || hprocKeyboard == NULL || hprocMouse == NULL) {
//        qDebug("Failed to load APMKeyHook.dll");
//        if(hinstAPMSharedDll == NULL)
//            qDebug("DLL not found");
//        else
//            qDebug("Couldn't find procedure (incompatible dll)");
//        return GetLastError();
//    }

//    HHOOK SetWindowsHookEx
//    (
//        int idHook,		// тип hook-точки, которая устанавливается
//        HOOKPROC lpfn,	// адрес подключаемой процедуры
//        HINSTANCE hMod,	// дескриптор экземпляра прикладной программы
//        DWORD dwThreadId	// идентификация потока, который устанавливает hook-точку
//    );

//    keyboardHook = SetWindowsHookEx(WH_KEYBOARD, hprocKeyboard, hinstAPMSharedDll, 0);
//    mouseHook = SetWindowsHookEx(WH_MOUSE, hprocMouse, hinstAPMSharedDll, 0);
//    if(keyboardHook == NULL || mouseHook == NULL) {
//        qDebug("Failed to register hooks");
//        return GetLastError();
//    }

    // если указано вести запись в лог файл, то создадим логгер (по умолчанию выключено)
    if(cfg->log_file)
        logger = new APMLogger(*cfg->log_file);

//     по умолчанию старт по хоткею
//    trigger->triggerStart();

    // устанавливаем таймер
//    timerId = SetTimer(NULL, 0, MEASURE_CYCLE_LENGTH-10, NULL);

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
//        return GetLastError();
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD, hprocKeyboard, hinstAPMSharedDll, 0);
    mouseHook = SetWindowsHookEx(WH_MOUSE, hprocMouse, hinstAPMSharedDll, 0);
    if(keyboardHook == NULL || mouseHook == NULL) {
        qDebug("Failed to register hooks");
//        return GetLastError();
    }

    timerId = SetTimer(NULL, 0, MEASURE_CYCLE_LENGTH-10, NULL);
    qDebug() << "meter started";
    max = 0;
    MSG msg = {0};
    while(!stop_flag)
    {
//        qDebug() << "get any message";
        // получаем сообщение
        BOOL bRet = GetMessage(&msg, NULL, 0, 0);
        if(bRet == 0 || bRet == -1)
            break;
//        qDebug() << "check timer";
        // если это сообщение таймера
        if(msg.message == WM_TIMER) {
//            qDebug() << "timer checked";
            // получаем текущий APM (текущий APM это APM вычисляемый из 20 последних действий)
            long current = measure->getCurrentAPM();
//            qDebug() << "Current:" << current << "Average:" << measure->getAverageAPM() << "Max:" << max;
            // если текущий APM больше максимального, то обновляем максимальный
            if(current > max)
                max = current;
            // выводим текущий APM, средний и максимальный
            measure->moveCurrentAPM();

            // если включено логирование, то делаем запись в лог
            if(logger != NULL)
                logger->addEntry(measure->getSnapshot());
        }
//        // проверяем на сообщение остановки APM сборщика
//        if(trigger->triggerStop(&msg)) {
//            break;
//        }

        //Эта функция пересылает сообщение оконной процедуре.
        //LRESULT DispatchMessage
        //(
        //    CONST MSG *lpmsg	// указатель на структуру с сообщением
        //);

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
    mainCycle();

}

void APMMeter::stop()
{
    stop_flag = true;
    measure->resetAllAPM();
}

long APMMeter::getTime()
{
//    APMLoggableSnapshot snap = measure->getSnapshot();
//    qDebug() << snap.snap.actions;
//    qDebug() << snap.snap.time;
//    qDebug() << snap.snap.apm;
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
    return max;
}
