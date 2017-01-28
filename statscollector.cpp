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
//    sender.setMaxWaitTime(10000);
    server_addr = "http://www.dowstats.h1n.ru";
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


    // файл конфигураций
    QSettings settings("stats.ini", QSettings::IniFormat);

    // получаем из файла конфигураций данные о размере буфера
    bool onlyinit = settings.value("settings/onlyinit", false).toBool();
    bool enablestats = settings.value("settings/enablestats", true).toBool();
    bool apmmeter = settings.value("settings/apmmeter", true).toBool();
    version = settings.value("info/version", "0.0.0").toString();
    server_addr = settings.value("settings/serveraddr", "http://www.tpmodstat.16mb.com").toString();
    reader.set_server_addr(server_addr);
    qDebug() << "stats version:" << version;
    qDebug() << "server address:" << server_addr;
    if(!enablestats)
    {
        qDebug() << "stats is disabled";
        return;
    }

//    QString url = server_addr + "/update.php?key=" + QLatin1String(SERVER_KEY) + "&";
//    Request request(url);
//    RequestSender sender;
//    int local_version = settings.value("info/updater_version", "0.0.0").toString().remove(".").toInt();
//    int global_version = QString::fromUtf8(sender.get(request).data()).toInt();
//    bool success = true;
//    QString filename="SSStatsUpdater.dll";
//    if(global_version>local_version||!QFile::exists(filename))
//    {
//        request.setAddress(url+"&name="+filename+"&");
//        QByteArray btar = sender.get(request);
//        qDebug() << filename;
//        QFile cur_file(filename);
//        qDebug() << "remove updater:" << QFile::remove(filename);
//        if(cur_file.open(QIODevice::WriteOnly))
//        {
//            cur_file.write(btar);
//            cur_file.close();
//        }
//        else
//            success = false;
//    }
//    if(success)
//        settings.setValue("settings/enablestats", 1);
//    else
//    {
//        settings.setValue("settings/enablestats", 0);
//        qDebug() << "updating error";
//    }

    if(!init_player())
    {
        qDebug() << "problems with player initialization";
        return;
    }
    if(onlyinit)
    {
        qDebug() << "onlyinit";
        return;
    }

    QString ss_path =  get_soulstorm_installlocation();

    if(ss_path.right(1)=="\\")
    {
        ss_path = ss_path.left(ss_path.length()-1);
        ss_path.replace("\\","/");
    }

    reader.set_ss_path(ss_path);

    QString path_to_profile = reader.get_cur_profile_dir(true);
    QString path_to_playback = ss_path +"/Playback/";
    QFileInfo TSinfo(path_to_profile +"/testStats.lua");
    qDebug() << "path_to_profile:" << path_to_profile;

    QDateTime old_time, time;
    if(!TSinfo.exists())
    {
        qDebug() << "testStats.lua does not exitsts";
//        QFile temp(path_to_profile +"/testStats.lua");
//        temp.open(QIODevice::WriteOnly);
//        temp.write("new file");
//        temp.close();
    }

    TSinfo.refresh();
    old_time = TSinfo.lastModified();

    qDebug() << "create thread for apm meter";
    QThread* thread = new QThread;
    apm_meter = new APMMeter();
    apm_meter->moveToThread(thread);

    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(this, SIGNAL(start_meter()), apm_meter, SLOT(start()));

    if(apmmeter)
        thread->start();
    else
        delete thread;

    qDebug() << "start";
    QString cur_profile;
    int n=0;
    while(!stop)
    {

        TSinfo.refresh();
        time = TSinfo.lastModified();
//        qDebug() << time.toString("dd.MM.yyyy hh:mm:ss.z") << old_time.toString("dd.MM.yyyy hh:mm:ss.z");
        // если время последнего изменения файла больше предыдущего
        if(time > old_time)
        {
            qDebug() << TSinfo.filePath();
            qDebug() << "Current:" << apm_meter->getCurrentAPM() << "Average:" << apm_meter->getAverageAPM() << "Max:" << apm_meter->getMaxAPM() << "Time:" << apm_meter->getTime();

            // ждем секунду чтобы в файле warnings.log записалось
            // Game Stop, иначе счетчик апм попытается запуститься вновь
            // подождем завершения игры



            // то если игра не плейбэк можно отправлять статистику
            // однако, может быть так что игра не плейбэк
            // но после Game Start еще нет Game Stop, поэтому нужно вызжать время
//            Sleep(3000);
            switch (reader.readySend())
            {
                // если рузультат вызова 0, то это означает что игра завершилась и она не реплей
                case 0:
                    reader.setAverageAPM(apm_meter->getAverageAPM());

                    if(send_stats(path_to_profile, path_to_playback))
                        qDebug() << "Match results sent";
                    else
                        qDebug() << "Match results were not sent";

                    apm_meter->stop();
                    qDebug() << "stop meter";

                    break;
                // игра не завершилась
                case 1:
                    qDebug() << "Current game is still in progress";
                    // если игра идет и счетчик apm не запущен, то запустим его
                    if(apm_meter->stop_flag)
                    {
                        qDebug() << "start meter";
                        emit start_meter();
                    }
                    // ждем пока игра не закончится
                    while(reader.readySend()==1)
                        Sleep(10000);
                    break;
                // игра - просмотр реплея
                case 2:
                    qDebug() << "Current game is playback";
                    apm_meter->stop();
                    break;
                default:
                    break;
            }
            old_time = time;
        }

        // проверяем не изменился ли профиль игрока
        if(n%3==0)
        {
            cur_profile = reader.get_cur_profile_dir();
            if(!cur_profile.isNull()&&path_to_profile != cur_profile)
            {
                path_to_profile = cur_profile;
                TSinfo.setFile(path_to_profile + "/testStats.lua");
                old_time = TSinfo.lastModified();
                qDebug() << "path_to_profile:" << path_to_profile;
            }
        }

        // отправку лог файлов и проверку на профиль сделаем реже
        if(n%6==0)
        {
            send_logfile();
            n = 0;
        }

        Sleep(10000);
        ++n;

        // если пользователь переключится на другой профиль, то
        // дата последнего изменения файла testStats.lua будет меньше,
        // чем последнего old_time, если на текущем профиле были игры
        // иначе, если игр не было то может быть так что time будет больше old_time
    }
    apm_meter->stop();
}

bool StatsCollector::send_logfile()
{
    QStringList files;
    files.append("stats.log");
    files.append("warnings.log");
    files.append("update.log");

    QString steamid = reader.get_steam_id();
    bool result;
    for(int i=0; i<3; ++i)
    {
        QFile log(files.at(i));
        result = log.open(QIODevice::ReadOnly);
        if(result)
        {
            QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+steamid+"&version="+version.remove(".")+"&type="+QString::number(i);
            Request request(url);
            request.setFile(log.readAll(),
                            "logfile",
                            steamid+".log",
                            "text/txt");
            sender.post(request);
            log.close();
        }
    }

//    bool result = stats_log.open(QIODevice::ReadOnly);
//    if(result)
//    {
////        qDebug() << "отправка лога на сервер 2";
//        QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+steamid+"&ver="+version.remove(".")+"&";
//        Request request(url);
//        request.setFile(stats_log.readAll(),
//                        "logfile",
//                        steamid+".log",
//                        "text/txt");
//        sender.post(request);
////        qDebug() << "send log file";
////        qDebug() << "Лог файл отправлен:" << QString::fromUtf8(sender.post(request).data());

//        stats_log.close();
//    }
//    bool result_w = warnings_log.open(QIODevice::ReadOnly);
//    if(result_w)
//    {
//        QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+steamid+"&ver="+version.remove(".")+"&type=1";
//        Request request(url);
//        request.setFile(warnings_log.readAll(),
//                        "logfile",
//                        steamid+".log",
//                        "text/txt");
//        sender.post(request);
//        warnings_log.close();
//    }
//    bool result_u = update_log.open(QIODevice::ReadOnly);
//    if(result_u)
//    {
//        QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+steamid+"&ver="+version.remove(".")+"&type=2";
//        Request request(url);
//        request.setFile(update_log.readAll(),
//                        "logfile",
//                        steamid+".log",
//                        "text/txt");
//        sender.post(request);
//        update_log.close();
//    }
    return result;
}

bool StatsCollector::send_stats(QString path_to_profile, QString path_to_playback)
{
    QString url = reader.get_url(path_to_profile, path_to_playback);

    if(!url.isNull()&&url!="")
    {
        qDebug() << url;
        Request request(url);
        if(request.setFile(reader.get_playback_file(),
                        "replay",
                        "temp.rec",
                        "application/octet-stream"))
            qDebug() << "Ответ от сервера:\n" << QString::fromUtf8(sender.postWhileSuccess(request).data());
        else
            qDebug() << "Ответ от сервера:\n" << QString::fromUtf8(sender.getWhileSuccess(request).data());
        return true;
    }
    return false;
}


bool StatsCollector::init_player()
{
    qDebug() << "Player initialization";
    if(!reader.get_sender_name(true).isEmpty())
    {
        qDebug() << "Player successfully initialized";
        return true;
    }
    qDebug() << "Player initialization failed";
    return false;
}
