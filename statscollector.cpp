#include "statscollector.h"
//#include <QTimer>
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDateTime>

#include <QDir>
#include <QSettings>



StatsCollector::StatsCollector()
{
    app = NULL;
    int argc = 0;
    app = new QCoreApplication(argc, NULL);
    log.installLog();
    sender.setMaxWaitTime(10000);
}

StatsCollector::~StatsCollector()
{
    log.finishLog();
    delete app;
}

QString StatsCollector::get_soulstorm_installlocation()
{
    QString ss_path;
    QSettings hklm("HKEY_LOCAL_MACHINE", QSettings::NativeFormat);
    for(int i=0; i<hklm.childGroups().size(); ++i)
    {
        if(hklm.childGroups().at(i).toLower()==QString("SOFTWARE").toLower())
        {

            QSettings software("HKEY_LOCAL_MACHINE\\"+hklm.childGroups().at(i), QSettings::NativeFormat);
            for(int j=0; j<software.childGroups().size(); ++j)
            {
                if(software.childGroups().at(j).toLower()==QString("THQ").toLower())
                {
                    QSettings thq(software.fileName()+"\\"+software.childGroups().at(j), QSettings::NativeFormat);
                    for(int k=0; k<thq.childGroups().size(); ++k)
                    {
                        if(thq.childGroups().at(k).toLower()==QString("Dawn of War - Soulstorm").toLower())
                        {
                            QSettings soulstorm(thq.fileName()+"\\"+thq.childGroups().at(k), QSettings::NativeFormat);
                            if(soulstorm.contains("installlocation"))
                                ss_path =  soulstorm.value("installlocation").toString();
                            if(!ss_path.isNull()) return ss_path;
                            else break;
                        }
                    }
                }
                if(software.childGroups().at(j).toLower()==QString("SEGA").toLower())
                {
                    QSettings sega(software.fileName()+"\\"+software.childGroups().at(j), QSettings::NativeFormat);
                    for(int k=0; k<sega.childGroups().size(); ++k)
                    {
                        if(sega.childGroups().at(k).toLower()==QString("Dawn of War - Soulstorm").toLower())
                        {
                            QSettings soulstorm(sega.fileName()+"\\"+sega.childGroups().at(k), QSettings::NativeFormat);
                            if(soulstorm.contains("installlocation"))
                                ss_path =  soulstorm.value("installlocation").toString();
                            if(!ss_path.isNull()) return ss_path;
                            else break;
                        }
                    }

                }
            }
            break;
        }
    }
    return QString();
}
void StatsCollector::start()
{

    if(!init_player())
    {
        qDebug() << "problems with player initialization";
        return;
    }
//    DWORD  MyThreadFunction( LPVOID lpParam )
//    {
//     QCoreApplication * app = NULL;
//     int argc = 0;
//     app = new QCoreApplication(argc, NULL);
////     QLibrary * dllClass =  new QLibrary();
//     app->exec();
//     return DWORD();
//    }

//    extern "C" QLIBRARY_EXPORT  void  instance()
//    {
//     LPDWORD lpThreadId = NULL;
//     CreateThread(
//      NULL,                   // default security attributes
//      0,                      // use default stack size
//      (LPTHREAD_START_ROUTINE)MyThreadFunction,       // thread function name
//      NULL,          // argument to thread function
//      0,                      // use default creation flags
//      lpThreadId);
//    }

//    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\thq\\dawn of war - soulstorm\\", QSettings::NativeFormat);

    QString ss_path =  get_soulstorm_installlocation();

    QDir temp_dir(ss_path);
//    temp_dir.cdUp();
    qDebug() << temp_dir.path();
    _cur_profile = reader.get_cur_profile_dir(ss_path);
    // если не удалось получить имя текущего профиля, то используем по умолчанию первый профиль
    if(_cur_profile=="") _cur_profile = "Profile1";
    QString path_to_profile = ss_path +"/Profiles/"+ _cur_profile;

    QFileInfo info(path_to_profile +"/testStats.lua");

    if(info.exists())
    {
        QDateTime old_time = info.lastModified();
        QDateTime time;
        while(!stop)
        {
            info.refresh();
            time = info.lastModified();
            if((time > old_time)&&(!reader.isPlayback()))
            {
                old_time = time;
                if(send_stats(path_to_profile))
                    qDebug() << "Match results sent";
                else
                    qDebug() << "Match results were not sent";
            }
//            QTime t;
//            t = QTime::fromString("15:45:35.88", "hh:mm:ss.z");

//            QTime t_start = QTime::fromString(reader.read_warnings_log("Starting mission", -3), "hh:mm:ss.z");
//            QTime t_end = QTime::fromString(reader.read_warnings_log("Ending mission", -3), "hh:mm:ss.z");
//            QTime t_abort = QTime::fromString(reader.read_warnings_log("Ending mission (Abort)", -3), "hh:mm:ss.z");
//            if(t_end>t_start&&t_end==t_abort)
//                send_stats("Disconnect");

//                game_exec = true;
//            if("Ending"==reader.read_warnings_log("Ending mission")&&
//                    "(Abort)"==)
            QThread::currentThread()->msleep(5000);
        }
    }
    else
        qDebug() << "file testStats.lua not exists";
}

bool StatsCollector::send_stats(QString path_to_profile)
{
    info = reader.get_game_info(path_to_profile);
    if(info!=0)
    {
//        info->get_url(server_addr"http://tpmodstat.16mb.com/connect.php?");
        QString url = info->get_url(server_addr);
//        qDebug() << url;
        if(!url.isNull())
        {
            Request request(url);
            if(sender.get(request).isNull())
                qDebug() << "Server returned empty data";
        }
        else
            return false;
    }
    else
        return false;
    return true;
}


bool StatsCollector::init_player()
{
    qDebug() << "Player initialization";
    if(!reader.get_sender_name(true).isNull())
    {
        qDebug() << "Player successfully initialized";
        return true;
    }
    return false;
}
