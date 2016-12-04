#include "statscollector.h"
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QObject>

StatsCollector::StatsCollector()
{
    app = NULL;
    int argc = 0;
    app = new QCoreApplication(argc, NULL);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));

    log.installLog();
    sender.setMaxWaitTime(10000);

}

StatsCollector::~StatsCollector()
{
    log.finishLog();
    delete apm_meter;
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

    QString ss_path =  get_soulstorm_installlocation();
    QDir temp_dir(ss_path);

    qDebug() << temp_dir.path();
    _cur_profile = reader.get_cur_profile_dir(ss_path);
    // если не удалось получить имя текущего профиля, то используем по умолчанию первый профиль
    if(_cur_profile=="") _cur_profile = "Profile1";
    QString path_to_profile = ss_path +"/Profiles/"+ _cur_profile;
    QString path_to_playback = ss_path +"/Playback/";
    QFileInfo info(path_to_profile +"/testStats.lua");

    qDebug() << "check testStats.lua exists";
    QDateTime old_time;
    QDateTime time;
    while(!info.exists())
    {
        info.refresh();
        QThread::currentThread()->msleep(5000);
    }
    old_time = info.lastModified();

    qDebug() << "create thread for apm meter";
    QThread* thread = new QThread;
    apm_meter = new APMMeter();
    apm_meter->moveToThread(thread);
//    QObject::connect(thread, SIGNAL(started()), apm_meter, SLOT(start()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(this, SIGNAL(start_meter()), apm_meter, SLOT(start()));
    thread->start();


//    QThread::currentThread()->msleep(5000);
    while(!stop)
    {
        int apm;
//        qDebug() << "start";
//        apm_meter->start();

        info.refresh();
        time = info.lastModified();
//        qDebug() << "check info updates";
        if(time > old_time)
        {
            apm = apm_meter->getAverageAPM();
            qDebug() << "Current:" << apm_meter->getCurrentAPM() << "Average:" << apm << "Max:" << apm_meter->getMaxAPM();
            if(!reader.isPlayback()/*&&!reader.isGameGoing()*/)
            {
//                qDebug() << "game is not playback and not going";
                reader.setAverageAPM(apm_meter->getAverageAPM());
                if(send_stats(path_to_profile, path_to_playback))
                    qDebug() << "Match results sent";
                else
                    qDebug() << "Match results were not sent";
                apm_meter->stop();
                qDebug() << "stop meter";
            }
            else
            {
                /* going or*/
                qDebug() << "Current game is playback";
            }
//                apm_meter->start();
//                if(!thread->isRunning()) thread->start();


            old_time = time;

            if(apm_meter->stop_flag&&reader.isGameGoing())
            {
                qDebug() << "start meter";
                emit start_meter();
            }
        }
//        qDebug() << apm_meter->stop_flag << reader.isGameGoing();

//        apm = apm_meter->getAverageAPM();
//        qDebug() << "VOT ONO! VOT ONO ZNACHENIE APM:" << apm << apm_meter->getCurrentAPM();

        QThread::currentThread()->msleep(10000);
    }
    apm_meter->stop();
}

bool StatsCollector::send_stats(QString path_to_profile, QString path_to_playback)
{
    info = reader.get_game_info(path_to_profile, path_to_playback);

    if(info!=0)
    {
        QString url = info->get_url(server_addr);
//        qDebug() << url;
        if(!url.isNull())
        {
            Request request(url);
//            QFile playback(path_to_playback+"temp.rec");
//            if(!playback.open(QIODevice::ReadWrite))
//            {
//                qDebug() << "Could not open temp.rec";
//                return 0;
//            }
            request.setFile(reader.get_playback_file());

//            if(sender.get(request).isNull())
//                qDebug() << "Server returned empty data";

            qDebug() << "Ответ от сервера:" << QString::fromUtf8(sender.post(request).data());
//            playback->close();
//            delete playback;
        }
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
