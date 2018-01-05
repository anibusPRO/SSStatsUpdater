#ifndef SYSTEMWIN32_H
#define SYSTEMWIN32_H

#include <windows.h>
#include <w32api.h>
#include <tlhelp32.h>

#include <QMap>
#include <QString>
#include <QStringList>
//#include <QMessageBox>
#include <QDebug>

#ifndef MAKEULONGLONG
#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw) & 0xFFFFFFFF))
#endif

#ifndef MAXULONGLONG
#define MAXULONGLONG ((ULONGLONG)~((ULONGLONG)0))
#endif

class systemWin32
{
public:
    systemWin32();
    ~systemWin32();
    void updateProcessList();
    bool findProcess(QString findProcName);
    static bool findProcess_2(QString findProcName);
    int findProcessCount(QString findProcName);
    static bool findProcessByWindowName(QString name);
    QString getProcessName(int idProcess);
    DWORD getProcessID(QString name);
    DWORD getProcessIDByWindowName(QString name);
    QStringList getAllProcessList();
    static bool CloseProcessMainThread(DWORD dwProcID);
    static DWORD getProcessMainThreadHandle(DWORD dwProcID);
    bool closeProcessByName(QString name);
private:
    QMap <DWORD, QString> win32sysMap;
};

#endif // SYSTEMWIN32_H
