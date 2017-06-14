#include <QCoreApplication>
//#include <signal.h>
#include "statscollector.h"
#include "systemwin32.h"
//#include "main.moc"

//class Button : public QObject {
//  Q_OBJECT
//public slots:
//  void stopStats() { qDebug() << "About to quit!"; }
//};

//void signalhandler(int sig){
//  if(sig==SIGINT){
//    qApp->quit();
//  }
//}

namespace boost{
    void throw_exception(std::exception const &e){}
}
int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    int res = 0;
    {
    QCoreApplication app(argc, argv);

    systemWin32 processes;
    if(processes.findProcessCount("SSStats.exe")>1)
    {
        qDebug() << "The SSStats.exe is already running";
        return res;
    }

//    QObject::connect(&app, SIGNAL(aboutToQuit()), &button, SLOT(doSomething()));
//    signal(SIGINT, signalhandler);

    StatsCollector stats;
    if(stats.start())
        res = app.exec();
    }
    qDebug() << "main stop";
    return res;
}
