#include <QCoreApplication>
#include "statscollector.h"
#include "systemwin32.h"

namespace boost{
    void throw_exception(std::exception const &e){}
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    systemWin32 processes;
    if(processes.findProcessCount("SSStats.exe")>1)
    {
        qDebug() << "The SSStats.exe is already running";
        return 0;
    }
    StatsCollector stats;

    stats.start();

    return 0;
}

