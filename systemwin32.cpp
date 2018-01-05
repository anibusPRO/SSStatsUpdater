#include "systemwin32.h"
#include <iostream>

systemWin32::systemWin32()
{
    win32sysMap.clear();

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap!=INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        Process32First(hSnap, &proc);
        do{
            win32sysMap[proc.th32ProcessID] = QString::fromWCharArray(proc.szExeFile);
        }while(Process32Next(hSnap, &proc));
        CloseHandle(hSnap);
    }else
        qDebug() << "CreateToolhelp32Snapshot: Error" << GetLastError();
}

systemWin32::~systemWin32()
{

}

bool systemWin32::CloseProcessMainThread(DWORD dwProcID)
{
  DWORD dwMainThreadID = 0;
  ULONGLONG ullMinCreateTime = MAXULONGLONG;

  HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap != INVALID_HANDLE_VALUE) {
    THREADENTRY32 th32;
    th32.dwSize = sizeof(THREADENTRY32);
    BOOL bOK = TRUE;
    for (bOK = Thread32First(hThreadSnap, &th32); bOK;
         bOK = Thread32Next(hThreadSnap, &th32)) {
      if (th32.th32OwnerProcessID == dwProcID) {
        HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,
                                    TRUE, th32.th32ThreadID);
        if (hThread) {
          FILETIME afTimes[4] = {FILETIME()};
          if (GetThreadTimes(hThread,
                             &afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) {
            ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime,
                                              afTimes[0].dwHighDateTime);
            if (ullTest && ullTest < ullMinCreateTime) {
              ullMinCreateTime = ullTest;
              dwMainThreadID = th32.th32ThreadID; // let it be main... :)
            }
          }else
            qDebug() << "GetThreadTimes: Error" << GetLastError();
          CloseHandle(hThread);
        }else
            qDebug() << "OpenThread: Error" << GetLastError();
      }
    }
#ifndef UNDER_CE
    CloseHandle(hThreadSnap);
#else
    CloseToolhelp32Snapshot(hThreadSnap);
#endif
  }
  else
      qDebug() << "CreateToolhelp32Snapshot: Error" << GetLastError();


  if (dwMainThreadID) {
    PostThreadMessage(dwMainThreadID, WM_QUIT, 0, 0); // close your eyes...
  }

  return (0 != dwMainThreadID);
}

DWORD systemWin32::getProcessMainThreadHandle(DWORD dwProcID)
{
  DWORD dwMainThreadID = 0;
  ULONGLONG ullMinCreateTime = MAXULONGLONG;

  HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (hThreadSnap != INVALID_HANDLE_VALUE) {
    THREADENTRY32 th32;
    th32.dwSize = sizeof(THREADENTRY32);
    BOOL bOK = TRUE;
    for (bOK = Thread32First(hThreadSnap, &th32); bOK;
         bOK = Thread32Next(hThreadSnap, &th32)) {
      if (th32.th32OwnerProcessID == dwProcID) {
        HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,
                                    TRUE, th32.th32ThreadID);
        if (hThread) {
          FILETIME afTimes[4] = {FILETIME()};
          if (GetThreadTimes(hThread,
                             &afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) {
            ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime,
                                              afTimes[0].dwHighDateTime);
            if (ullTest && ullTest < ullMinCreateTime) {
              ullMinCreateTime = ullTest;
              dwMainThreadID = th32.th32ThreadID; // let it be main... :)
            }
          }else
            qDebug() << "GetThreadTimes: Error" << GetLastError();
          CloseHandle(hThread);
        }else
            qDebug() << "OpenThread: Error" << GetLastError();
      }
    }
#ifndef UNDER_CE
    CloseHandle(hThreadSnap);
#else
    CloseToolhelp32Snapshot(hThreadSnap);
#endif
  }
  else
      qDebug() << "CreateToolhelp32Snapshot: Error" << GetLastError();

  return dwMainThreadID;
}

void systemWin32::updateProcessList()
{
    win32sysMap.clear();

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == INVALID_HANDLE_VALUE)
    {
        qDebug() << "CreateToolhelp32Snapshot: Error" << GetLastError();
        return;
    }

    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnap, &proc);
    do{
        win32sysMap[proc.th32ProcessID] = QString::fromWCharArray(proc.szExeFile);
    }while(Process32Next(hSnap, &proc));

    CloseHandle(hSnap);
}

// принимает имя процесса, возвращает true, если процесс запущен
bool systemWin32::findProcess_2(QString findProcName)
{
    WCHAR temp[findProcName.size()+1]={0};
    findProcName.toWCharArray(temp);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == INVALID_HANDLE_VALUE)
    {
        qDebug() << "CreateToolhelp32Snapshot: Error" << GetLastError();
        return true;
    }

    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnap, &proc);
    do{
        if(wcscmp(proc.szExeFile, temp)==0)
        {
            CloseHandle(hSnap);
            return true;
        }
    }while(Process32Next(hSnap, &proc));
    return false;
}

// принимает имя процесса, возвращает true, если процесс запущен
bool systemWin32::findProcess(QString findProcName)
{
    return win32sysMap.values().contains(findProcName);
}

// считает количество процессов с данным именем и возвращает результат
int systemWin32::findProcessCount(QString findProcName)
{
    return win32sysMap.values().count(findProcName);
}

// получить имя процесса по ID-у
QString systemWin32::getProcessName(int idProcess)
{
    return win32sysMap.value(idProcess);
}

DWORD systemWin32::getProcessID(QString name)
{
    return win32sysMap.key(name);
}

bool systemWin32::closeProcessByName(QString name)
{
    bool result = true;
    foreach (DWORD key, win32sysMap.keys()) {
        if(win32sysMap.value(key)==name&&!CloseProcessMainThread(key))
            result = false;
    }
    return result;
}

// получить список всех процессов
QStringList systemWin32::getAllProcessList()
{
    return win32sysMap.values();
}

DWORD systemWin32::getProcessIDByWindowName(QString name)
{
    WCHAR temp[name.size()+1]={0};
    name.toWCharArray(temp);
    HWND hWnd = FindWindow(NULL, temp);
    DWORD PID;
    if(hWnd==NULL){
        return 0;
    }
    GetWindowThreadProcessId(hWnd, &PID);
    return PID;
}

bool systemWin32::findProcessByWindowName(QString name)
{
    WCHAR temp[name.size()+1]={0};
    name.toWCharArray(temp);
    HWND hWnd = FindWindow(NULL, temp);
    DWORD PID;
    if(hWnd==NULL){
        return false;
    }
    GetWindowThreadProcessId(hWnd, &PID);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == INVALID_HANDLE_VALUE)
    {
        qDebug() << "CreateToolhelp32Snapshot: Error" << GetLastError();
        return true;
    }

    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnap, &proc);
    do{
        if(proc.th32ProcessID==PID)
        {
            CloseHandle(hSnap);
            return true;
        }
    }while(Process32Next(hSnap, &proc));
    return false;
}
