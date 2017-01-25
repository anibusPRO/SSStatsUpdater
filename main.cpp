#include <windows.h>
#include <thread>

#include <iostream>

#include "statscollector.h"

static StatsCollector stats_c;

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, PVOID fImpLoad)
{

    switch(fdwReason)
    {
        // при подключении библиотеки
        case DLL_PROCESS_ATTACH:
        {
            std::thread thr(&StatsCollector::start, &stats_c);
            thr.detach();
        }
            break;
        // при отключении библиотеки
        case DLL_PROCESS_DETACH:
                stats_c.stop = true;
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        default:
            break;
    }
    return TRUE;
}

// эта функция необходима для того чтобы программа загружала данную библиотеку
// больша функция никак не используется и ни откуда не вызывается
Q_DECL_EXPORT void my_func(void)
{
}
