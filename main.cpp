#include <QAbstractEventDispatcher>
#include "statscollector.h"
#include "systemwin32.h"

namespace boost{
    void throw_exception(std::exception const &e){(void)e;}
}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    int res = 0;

    QCoreApplication app(argc, argv);

    systemWin32 processes;
    if(processes.findProcessCount("SSStats.exe")>1)
    {
        qDebug() << "The SSStats.exe is already running";
        return res;
    }
    StatsCollector stats;

    if(stats.start())
        res = app.exec();

    return res;
}
