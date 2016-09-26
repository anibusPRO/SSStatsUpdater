#include "statscollector.h"
#include <windows.h>
#include <thread>

#include <iostream>


////#define EXPORT extern “C” __declspec (dllexport)
////EXPORT void CALLBACK my_func();

////using namespace std;

//#define APP_NAME "Soulstorm Stats Reader"
//#define APP_VERSION "1.0"
//#define ORG_NAME "New"
//#define ORG_DOMAIN "loa92@mail.ru"



////#define __DATE__
////#define __TIME__

//// Данные для ведения логов
//static QTextStream* logStream;
//static QFile* logFile;
//static QThread* main_thread;
static StatsCollector stats_c;

//// Типы сообщений
//static const char* msgType[] =
//{
//    "(II) ", // Info
//    "(WW) ", // Warning
//    "(EE) ", // Error
//    "(FF) " // Fatal error
//};

//const QString TextDescription = QObject::tr(
//"%1 %2\n"
//"Built on " __DATE__ " at " __TIME__ ".\n"
//"Based on Qt %3.\n"
//"Copyright %4. All rights reserved.\n"
//"See also %5\n")
//.arg(QLatin1String(APP_NAME), QLatin1String(APP_VERSION),
//QLatin1String(QT_VERSION_STR), QLatin1String(ORG_NAME), QLatin1String(ORG_DOMAIN)
//);

//// Вывод логов в файл
//void customMessageHandler(QtMsgType type, const char* msg);
//// Создание файла для логов
//void installLog();
//// Закрытие файла логов
//void finishLog();
//// Информация об ОС
//QString getOSInfo();


BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, PVOID fImpLoad)
{

    switch(fdwReason)
    {
//    int argc;
//    char** argv;
//    argc = 0;
    case DLL_PROCESS_ATTACH:
    {
//    QCoreApplication app(argc, argv);

//    main_thread = new QThread();
    // Под Windows устанавливаем кодировку cp1251
//    #ifdef Q_WS_WIN
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("Windows-1251"));
//    // Под остальными ОС - utf8
//    #else
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
//    #endif

//    installLog();

//    QObject::connect(main_thread, SIGNAL(started()) , &stats_c, SLOT(start()));
//    QObject::connect(main_thread, SIGNAL(finished()), main_thread, SLOT(deleteLater()));

//    stats_c.moveToThread(main_thread);

//        stats_c.testStats_path = "/Profiles/Profile1/testStats.lua";
        stats_c.server_addr = "http://tpmodstat.16mb.com/connect.php?";
        std::thread thr(&StatsCollector::start, &stats_c);
        thr.detach();

//        stats_c.start();


    }
        break;

    case DLL_PROCESS_DETACH:
//        if(main_thread->isRunning())
//        {
            stats_c.stop = true;
//            finishLog();
//            main_thread->wait();
//        }
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


//void customMessageHandler(QtMsgType type, const char* msg)
//{
//    std::cout << msgType[type] << msg << std::endl;
//    if(logStream && logStream->device())
//    {
//        *logStream << msgType[type] << msg << endl;
//    }
//}

//void installLog()
//{
//    logFile = new QFile("stats.log");
//    if(logFile->open(QFile::WriteOnly | QIODevice::WriteOnly | QIODevice::Unbuffered))
//    logStream = new QTextStream(logFile);

//    #ifdef Q_WS_WIN
//    logStream->setCodec("Windows-1251");
//    // Под остальными ОС - utf8
//    #else
//    logStream->setCodec("utf-8");
//    #endif

//    // Запись заголовка с информацией о запуске
//    if(logStream && logStream->device())
//    {
//        *logStream << endl << TextDescription << endl;
//        *logStream << QString("Markers: (II) informational, (WW) warning,") << endl;
//        *logStream << QString("(EE) error, (FF) fatal error.") << endl;
//        *logStream << getOSInfo() << endl;
//        *logStream << QString("Runned at %1.").arg(QDateTime::currentDateTime().toString()) << endl << endl;
//    }

//    qInstallMsgHandler(customMessageHandler);

//    qDebug("Success opening log file");
//}

//void finishLog()
//{
//    qDebug("Success closing log file");

//    delete logStream;
//    logStream = 0;
//    delete logFile;
//    logFile = 0;

//    qInstallMsgHandler(0);
//}

//QString getOSInfo()
//{
//    QString infoStr("Current Operating System: %1");
//    #ifdef Q_OS_WIN
//    switch(QSysInfo::windowsVersion())
//    {
//        case QSysInfo::WV_NT: return infoStr.arg("Windows NT");
//        case QSysInfo::WV_2000: return infoStr.arg("Windows 2000");
//        case QSysInfo::WV_XP: return infoStr.arg("Windows XP");
//        case QSysInfo::WV_2003: return infoStr.arg("Windows 2003");
//        case QSysInfo::WV_VISTA: return infoStr.arg("Windows Vista");
//        case QSysInfo::WV_WINDOWS7: return infoStr.arg("Windows Seven");
//        case QSysInfo::WV_WINDOWS8: return infoStr.arg("Windows 8");
//        case QSysInfo::WV_WINDOWS8_1 : return infoStr.arg("Windows 8.1");
//        default: return infoStr.arg("Windows");
//    }
//    #endif // Q_OS_WIN

//    #ifdef Q_OS_MAC
//    switch(QSysInfo::MacintoshVersion())
//    {
//        case QSysInfo::MV_CHEETAH: return infoStr.arg("Mac OS X 1.0 Cheetah");
//        case QSysInfo::MV_PUMA: return infoStr.arg("Mac OS X 1.1 Puma");
//        case QSysInfo::MV_JAGUAR: return infoStr.arg("Mac OS X 1.2 Jaguar");
//        case QSysInfo::MV_PANTHER: return infoStr.arg("Mac OS X 1.3 Panther");
//        case QSysInfo::MV_TIGER: return infoStr.arg("Mac OS X 1.4 Tiger");
//        case QSysInfo::MV_LEOPARD: return infoStr.arg("Mac OS X 1.5 Leopard");
//        case QSysInfo::MV_SNOWLEOPARD: return infoStr.arg("Mac OS X 1.6 Snow Leopard");
//        default: return infoStr.arg("Mac OS X");
//    }
//    #endif // Q_OS_MAC

//    #ifdef Q_OS_LINUX
//    #ifdef LINUX_OS_VERSION
//    return infoStr.arg(LINUX_OS_VERSION);
//    #else
//    return infoStr.arg("Linux");
//    #endif // LINUX_OS_VERSION
//    #endif // Q_OS_LINUX
//}


__declspec(dllexport)void my_func()
{
//    QFile temp("temp.txt");
//    if(temp.open(QFile::WriteOnly | QIODevice::WriteOnly | QIODevice::Unbuffered))
//    {
//        temp.write("Privet");
//        temp.close();
//    }
}
