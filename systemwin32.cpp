#include "systemwin32.h"

systemWin32::systemWin32()
{
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == NULL)
    {
//        QMessageBox::critical(0, "Error!", "Error Load ToolHelp", QMessageBox::Close);
        qDebug() << "Error Load ToolHelp";
        return;
    }

    PROCESSENTRY32 proc = { sizeof(proc) };

    if (Process32First(hSnap, &proc))
    {
        QString filename;

        filename = copyToQString(proc.szExeFile);
        win32sysMap[proc.th32ProcessID] = filename;

        while (Process32Next(hSnap, &proc))
        {
            filename = copyToQString(proc.szExeFile);
            win32sysMap[proc.th32ProcessID] = filename;
        }
    }
}

systemWin32::~systemWin32()
{
    CloseHandle(hSnap);
}

void systemWin32::updateProcessList()
{
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    win32sysMap.clear();

    if (hSnap == NULL)
    {
        qDebug() << "Error Load ToolHelp";
        return;
    }

    PROCESSENTRY32 proc = { sizeof(proc) };

    if (Process32First(hSnap, &proc))
    {
        QString filename;

        filename = copyToQString(proc.szExeFile);
        win32sysMap[proc.th32ProcessID] = filename;

        while (Process32Next(hSnap, &proc))
        {
            filename = copyToQString(proc.szExeFile);
            win32sysMap[proc.th32ProcessID] = filename;
        }
    }
}

// принимает имя процесса, возвращает true, если процесс запущен
bool systemWin32::findProcess(QString findProcName)
{
    QMapIterator<int, QString> i(win32sysMap);
    while (i.hasNext())
    {
        i.next();
        if (i.value().contains(findProcName, Qt::CaseInsensitive))
        {
//            qDebug() << i.value();
            return true;
        }
    }

    return false;
}

// считает количество процессов с данным именем и возвращает результат
int systemWin32::findProcessCount(QString findProcName)
{
    int counter = 0;
    QMapIterator<int, QString> i(win32sysMap);
    while (i.hasNext())
    {
        i.next();
        if (i.value() == findProcName)
            counter++;
    }

    return counter;
}

// получить имя процесса по ID-у
QString systemWin32::getProcessName(int idProcess)
{
    return win32sysMap[idProcess];
}

// имена процессов WinAPI представлены как массив WCHAR. Для удобства - возврат в QString
// хэз, может как-нить по другому можно сделать
QString systemWin32::copyToQString(WCHAR array[MAX_PATH])
{
    QString string;
    int i = 0;

    while (array[i] != 0)
    {
        string[i] = array[i];
        i++;
    }
    return string;
}

// получить список всех процессов
QStringList systemWin32::getAllProcessList()
{
    return win32sysMap.values();
}
