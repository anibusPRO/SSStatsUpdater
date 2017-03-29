#include <QCoreApplication>
#include "statscollector.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    StatsCollector stats;

    stats.start();

    return 0;
}

