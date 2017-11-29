#include "monitor.h"
//#include <iostream>
#include <psapi.h>//информация о памяти процесса
//#include <fstream>//для вывода в документ адресов и значений по ним
#include <QDebug>
#include <QRegExp>
#include <QFileInfo>

Monitor::Monitor()
{

}

Monitor::~Monitor()
{

}

int Monitor::GetSteamPlayersInfo() {
    qDebug() << "Get Steam Players Info";

    finished = false;
//    PlayersInfo.clear();
    QByteArray buffer(30400, 0);

    HWND hWnd = FindWindow(NULL, L"Dawn of War: Soulstorm");
    DWORD PID;
    GetWindowThreadProcessId(hWnd, &PID);

    // Получение дескриптора процесса
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                       PROCESS_VM_READ, FALSE, PID);
    if(hProcess==nullptr){
        qDebug() << "OpenProcess Error" << GetLastError();
        finished = true;
        return GetLastError();
    }
    long ptr1Count = 0x00000000; // адресс после недоступной зоны
    MEMORY_BASIC_INFORMATION b; // Объявляем структуру
    LPCVOID ptr1 = (LPCVOID)ptr1Count;

    int counter = 1;
    SIZE_T total_size = 0;


//    qDebug() << "Process handle" << hProcess;
//    TCHAR pname[MAX_PATH], szModName[MAX_PATH];
//    if(!GetProcessImageFileName(hProcess, pname, sizeof(pname))){
//        qDebug() << "GetProcessImageFileName" << GetLastError();
//        finished = true;
//        CloseHandle(hProcess);
//        return GetLastError();
//    }
//    HMODULE hMods[1024];
//    DWORD cbNeeded;
//    MODULEINFO mInfo;
//    QString QSpname = QFileInfo(QString::fromStdWString(std::wstring(pname))).fileName();

//    if(!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
//    {
//        qDebug() << "EnumProcessModules" << GetLastError();
//        finished = true;
//        CloseHandle(hProcess);
//        return GetLastError();
//    }

//    QString temp;
//    for(int i=0; i < (cbNeeded / sizeof(HMODULE)); i++)
//        if(GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName)/sizeof(TCHAR))){
//            temp = QFileInfo(QString::fromStdWString(std::wstring(szModName))).fileName();
//            if (QSpname.contains(temp)){
//                if(!GetModuleInformation(hProcess, hMods[i], &mInfo, sizeof(mInfo)))
//                    qDebug() << "GetModuleInformation" << GetLastError();
//                break;
//            }
//        }
//        else
//            qDebug() << "GetModuleFileNameEx" << GetLastError();

//    qDebug() << temp;
//    qDebug() << "lpBaseOfDll" << mInfo.lpBaseOfDll;
//    qDebug() << "SizeOfImage" << mInfo.SizeOfImage;
//    qDebug() << "buffer.size()" << buffer.size();
//    qDebug() << "buffer.data()" << (PVOID)buffer.data();

    while (ptr1Count <= 0x7FFE0000) // До конца виртуальной памяти для данного процесса
    {
        VirtualQueryEx(hProcess, ptr1, &b, sizeof(b));

        if(b.State==MEM_COMMIT&&
            b.AllocationProtect==PAGE_READWRITE&&
            b.Protect==PAGE_READWRITE&&
            b.Type==MEM_PRIVATE&&
            0x1000<=b.RegionSize&&b.RegionSize<0xFFF000)
        {
            total_size += b.RegionSize;

            for (PVOID readAddr = b.BaseAddress; readAddr < b.BaseAddress+b.RegionSize; readAddr += buffer.size() - 200) {
                SIZE_T bytesRead = 0;

                // если функция вернула не ноль, то продолжим цикл
                if(!ReadProcessMemory(hProcess, (PVOID)readAddr, (PVOID)buffer.data(), buffer.size(), &bytesRead))
                    continue;
                if(bytesRead<200)
                    continue;
                for (int i = 0; i < bytesRead - 200; i++) {
                    bool match = false;
                    for (int j = 0; j < sizeof(steamHeader); j++)
                        if (buffer[i + j] != steamHeader[j]){
                            match = false;
                            break;
                        }else match = true;

                    if (!match)
                        continue;

                    int nickPos = i + 56;
                    if (buffer[nickPos] < 50 &&
                            buffer[nickPos] > 0 &&
                            buffer[nickPos + 1] == 0 &&
                            buffer[nickPos + 2] == 0 &&
                            buffer[nickPos + 3] == 0 &&
                            buffer[nickPos - 1] == 0 &&
                            buffer[nickPos - 2] == 0 &&
                            buffer[nickPos - 3] == 0 &&
                            buffer[nickPos - 4] == 0 &&
                            buffer[nickPos+4+buffer[nickPos]*2] == 0 &&
                            buffer[nickPos+4+buffer[nickPos]*2+1] == 0 &&
                            buffer[nickPos+4+buffer[nickPos]*2+2] == 0 &&
                            buffer[nickPos+4+buffer[nickPos]*2+3] == 0) {
                        QString steamIdStr = QString::fromUtf16((ushort*)buffer.mid(i + 18, 34).data()).left(17);
                        if(!steamIdStr.contains(QRegExp("^[0-9]{17,17}$")))
                            continue;

                        QString nick = QString::fromUtf16((ushort*)buffer.mid(nickPos + 4, buffer[nickPos] * 2).data()).left(buffer[nickPos]);
                        if(!PlayersInfo.contains(nick)||PlayersInfo.value(nick)!=steamIdStr)
                            PlayersInfo.insert(nick, steamIdStr);

                    }
                }
            }
        }
        ptr1Count = ptr1Count + (int)b.RegionSize;
        ptr1 = (PVOID)ptr1Count;
        ++counter;
    }

    finished = true;
    DWORD mem_size = total_size/1024/1024;
    qDebug() << "Total memory size:" << mem_size << "mb";
    CloseHandle(hProcess);
    return 0;
}
