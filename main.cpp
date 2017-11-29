#include <QCoreApplication>
//#include <QTextCodec>
//#include <QtGui\QApplication>
#include <QAbstractEventDispatcher>
#include "statscollector.h"
#include "systemwin32.h"

//#include <iostream>
//#include <io.h>
//#include <fcntl.h>



//#include <initializer_list>
//#include <signal.h>
//#include <unistd.h>

//void ignoreUnixSignals(std::initializer_list<int> ignoreSignals) {
//    // all these signals will be ignored.
//    for (int sig : ignoreSignals)
//        signal(sig, SIG_IGN);
//}

//void catchUnixSignals(std::initializer_list<int> quitSignals) {
//    auto handler = [](int sig) -> void {
//        // blocking and not aysnc-signal-safe func are valid
////        printf("\nquit the application by signal(%d).\n", sig);
//        qDebug() << "quit the application by signal" << sig;
//        QCoreApplication::quit();
//    };

//    sigset_t blocking_mask;
//    sigemptyset(&blocking_mask);
//    for (auto sig : quitSignals)
//        sigaddset(&blocking_mask, sig);

//    struct sigaction sa;
//    sa.sa_handler = handler;
//    sa.sa_mask    = blocking_mask;
//    sa.sa_flags   = 0;

//    for (auto sig : quitSignals)
//        sigaction(sig, &sa, nullptr);
//}

//#include "main.moc"

//class Button : public QObject {
//  Q_OBJECT
//public slots:
//  void stopStats() { qDebug() << "About to quit!"; }
//};

void signalhandler(int sig){
    qDebug() << "signalhandler" << sig;
    if(sig==SIGINT||sig==SIGTERM){
        qApp->quit();
    }
}

bool eventFilter(void* message)
{
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_QUIT ||
        msg->message == WM_CLOSE ||
        msg->message == WM_QUERYENDSESSION ||
        msg->message == WM_ENDSESSION)
    {
        qDebug() << "eventFilter" << msg->message;
        QCoreApplication::quit();
    }
    return false;
}

namespace boost{
    void throw_exception(std::exception const &e){(void)e;}
}
int main(int argc, char *argv[])
{
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    int res = 0;
    {
        QCoreApplication app(argc, argv);
//        qDebug() << "Запуск";
//        std::cout << "Запуск\n";
//        printf("Запуск\n");

        systemWin32 processes;
        if(processes.findProcessCount("SSStats.exe")>1)
        {
            qDebug() << "The SSStats.exe is already running";
            return res;
        }
        StatsCollector stats;
//        QAbstractEventDispatcher::instance()->setEventFilter(eventFilter);
//        QObject::connect(&app, SIGNAL(aboutToQuit()), &stats, SLOT(exitHandler()));
//        signal(SIGQUIT, signalhandler);
//        signal(SIGINT, signalhandler);
//        signal(SIGTERM, signalhandler);
//        signal(SIGINT, signalhandler);
//        signal(SIGILL, signalhandler);
//        signal(SIGABRT_COMPAT, signalhandler);
//        signal(SIGFPE, signalhandler);
//        signal(SIGSEGV, signalhandler);
//        signal(SIGTERM, signalhandler);
//        signal(SIGBREAK, signalhandler);
//        signal(SIGABRT, signalhandler);
//        signal(SIGABRT2, signalhandler);

//        signal(SIGHUP, signalhandler);
//        catchUnixSignals({SIGINT, SIGTERM});

        if(stats.start())
            res = app.exec();
    }
    qDebug() << "main stop";
    return res;
}
