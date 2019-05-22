#include <QCoreApplication>
#include "statsupdater.h"
//#include "form.h"

namespace boost{
    void throw_exception(std::exception const &e){}
}
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    StatsUpdater updater;
    updater.start();
//    int res = 0;
//    {
//        QApplication app(argc, argv);
//        Form window;
//        if(QFile::exists("t_SSStatsUpdater.exe")&&QFile::exists("SSStatsUpdaterupdater.bat"))
//            return 0;
//        if(argc>=2&&argv[1]=="-s"){
//            StatsUpdater stats_u;
//            stats_u.start();
//        }else{
//            window.show();
//            qDebug() << "form";
//        }
//        res = app.exec();
//    }
    return 0;
//    return app.exec();
}

