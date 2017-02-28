#include "statscollector.h"
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QObject>
#include <fstream>
#include <string>
#include <iostream>
#include <QCryptographicHash>

StatsCollector::StatsCollector(QObject* pobj /*=0*/)
    :QObject(pobj)
{
    app = NULL;
    int argc = 0;
    app = new QCoreApplication(argc, NULL);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));

    log.installLog();
    server_addr = "http://www.dowstats.h1n.ru";

    QThread *thread = new QThread( );

    sender = new RequestSender();
    sender->moveToThread(thread);
    reader = new GameInfoReader();

    connect(sender, SIGNAL(done(const QUrl&, const QByteArray&)),
    this, SLOT(slotDone(const QUrl&, const QByteArray&)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), sender, SLOT(deleteLater()));
    connect(this, SIGNAL(sendfile(QString,QString,QString,QByteArray)),
            sender, SLOT(upload(QString,QString,QString,QByteArray)));
    connect(this, SIGNAL(get(QString)), sender, SLOT(download(QString)));
    thread->start();
}

void StatsCollector::start()
{


    // файл конфигураций
    QSettings settings("stats.ini", QSettings::IniFormat);

    // получаем из файла конфигураций данные о размере буфера
    bool enablestats = settings.value("settings/enablestats", true).toBool();
//    bool apmmeter = settings.value("settings/apmmeter", true).toBool();
    bool sendreplays = settings.value("settings/sendreplays", true).toBool();
    version = settings.value("info/version", "0.0.0").toString();
    server_addr = settings.value("settings/serveraddr", "http://www.dowstats.h1n.ru/").toString();
    qDebug() << "stats version:" << version;
    qDebug() << "server address:" << server_addr;
    if(!enablestats)
    {
        qDebug() << "stats is disabled";
        return;
    }


//    QString filename="SSStatsUpdater.dll";
//    QString url = server_addr + "/update.php?key=" + QLatin1String(SERVER_KEY) ;
//    Request request(url + "&name=ssstats.md5&");
//    QByteArray btar = sender->get(request);
//    QString md5 = QString::fromUtf8(btar.data());
//    qDebug() << "stats updater md5 checking...";
//    if(!md5.contains(calcMD5(filename)))
//    {
//        // код который получает программу для автообновления, создает bat файл и запускает процесс, его выполняющий
//        // bat файл удаляет старую программу для автообновления, а затем переименовывает скачанную новую
//        request.setAddress(url+"&name="+filename+"&");
//        bool success = true;
//        QByteArray btar = sender->getWhileSuccess(request);
//        qDebug() << filename << "successfully downloaded";

//        QFile cur_file("SSStatsUpdaterTemp.dll");
//        if(cur_file.open(QIODevice::WriteOnly))
//        {
//            cur_file.write(btar);
//            cur_file.close();
//        }
//        else
//            success = false;
//        if(success)
//            updateUpdater();
//    }

    qDebug() << "Player initialization";
    if(!init_player())
    {
        qDebug() << "Player initialization failed";
        return;
    }
    // это должно посылаться сервером
//    qDebug() << "Player successfully initialized";


    QString ss_path =  get_soulstorm_installlocation();
    reader->set_ss_path(ss_path);
    reader->set_accounts(accounts);
    QString path_to_profile = reader->get_cur_profile_dir();
    QFileInfo TSinfo(path_to_profile +"/testStats.lua");
    qDebug() << "path_to_profile:" << path_to_profile;

    QDateTime old_time, last_time;
    if(!TSinfo.exists())
    {
        qDebug() << "testStats.lua does not exitsts";
        QFile temp(path_to_profile +"/testStats.lua");
        temp.open(QIODevice::WriteOnly);
        temp.write("new file");
        temp.close();
    }

    TSinfo.refresh();
    old_time = TSinfo.lastModified();

    QThread* thread = new QThread;
    apm_meter = new APMMeter();
    apm_meter->moveToThread(thread);

    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(this, SIGNAL(start_meter()), apm_meter, SLOT(start()));

    // это надо доработать
//    if(apmmeter)
    thread->start();
//    else
//        delete thread;

    qDebug() << "start";
    QString cur_profile, params;
//    int n=0;long avrAPM, TotalActions;
    while(!stop)
    {
        TSinfo.refresh();
        last_time = TSinfo.lastModified();
        // если время последнего изменения файла больше предыдущего
        if(last_time > old_time)
        {
            switch (reader->readySend())
            {
                // если рузультат вызова 0, то это означает что игра завершилась и она не реплей
                case 0:
                    apm_meter->stop();
                    qDebug() << "Game Stop";
//                    TotalActions = apm_meter->getTotalActions();
//                    avrAPM = apm_meter->getAverageAPM();
//                    qDebug() << "Current:" << apm_meter->getCurrentAPM()
//                             << "Average:" << avrAPM
//                             << "Max:" << apm_meter->getMaxAPM()
//                             << "Time:" << (double)apm_meter->getTime()/1000.0f/60.0f << "min"
//                             << "Total Actions:" << TotalActions;

                    reader->setTotalActions(apm_meter->getTotalActions());
                    params = reader->get_game_info(path_to_profile);
//                    if(!params.isEmpty())
//                    {
//                        Request request(server_addr+"/connect.php?"+params);
//                        if(request.setFile(reader->get_playback_file(),
//                                        "replay",
//                                        reader->get_playback_name(),
//                                        "application/octet-stream"))
//                            qDebug() << "Отправка статистики с реплеем. Ответ от сервера:\n" << QString::fromUtf8(sender->postWhileSuccess(request).data());
//                        else
//                            qDebug() << "Отправка статистики без реплея. Ответ от сервера:\n" << QString::fromUtf8(sender->getWhileSuccess(request).data());
//                    }
                    qDebug() << "debug 3";
                    if(!params.isEmpty())
                        if(sendreplays)
                            emit sendfile(server_addr+"/connect.php?"+params,
                                          reader->get_playback_name(),
                                          "application/octet-stream",
                                          reader->get_playback_file());
                        else
                            emit get(server_addr+"/connect.php?"+params);
                    qDebug() << "debug 4";
                    break;
                // игра не завершилась
                case 1:
                    qDebug() << "Game Start";
                    // если игра идет и счетчик apm не запущен, то запустим его
                    if(apm_meter->stop_flag)
                        emit start_meter();

                    // ждем пока игра не закончится
                    while(reader->readySend()!=0)
                        Sleep(5000);
                    break;
                // игра - просмотр реплея
                case 2:
                    qDebug() << "Current game is playback";
                    apm_meter->stop();
                    break;
                default:
                    break;
            }
            old_time = last_time;
        }
        else
        {
            cur_profile = reader->get_cur_profile_dir();
            if((!cur_profile.isNull())&&path_to_profile != cur_profile)
            {
                path_to_profile = cur_profile;
                TSinfo.setFile(path_to_profile + "/testStats.lua");
                old_time = TSinfo.lastModified();
                qDebug() << "path_to_profile:" << path_to_profile;
            }
            // отправим лог файлы
            send_logfile();
            Sleep(10000);
//            ++n;
        }
    }
    if(thread->isRunning())
    {
        apm_meter->stop();
        thread->wait();
    }
}

bool StatsCollector::send_logfile()
{
    QStringList files;
    files << "stats.log" <<
             "warnings.log";
    QString steamid = reader->get_steam_id();

    if(steamid.isEmpty())
        steamid = accounts.values().first();

    if(steamid.isEmpty()) return false;
    bool result;
    for(int i=0; i<2; ++i)
    {
        QFile log(files.at(i));
        result = log.open(QIODevice::ReadOnly);
        if(result)
        {
            QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+steamid+"&version="+version.remove(".")+"&type="+QString::number(i);
//            Request request(url);
//            request.setFile(log.readAll(),
//                            "logfile",
//                            steamid+".log",
//                            "text/txt");
//            sender->post(request);
            emit sendfile(url, steamid+".log", "text/txt", log.readAll());
            log.close();
        }
    }
    return result;
}

int StatsCollector::updateUpdater()
{
    std::ofstream out("updater.bat");
    out << "@echo off\n";
    out << ":try\n";
    out << "del SSStatsUpdater.dll\n";
    out << "if exist SSStatsUpdater.dll goto try\n";
    out << "ren SSStatsUpdaterTemp.dll SSStatsUpdater.dll\n";
    out << "del updater.bat";
    out.close();

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;

    ZeroMemory( &si, sizeof(si) );
    ZeroMemory( &pi, sizeof(pi) );
    si.cb = sizeof(si);
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW;

    QString pathexe = "C:\\Windows\\system32\\cmd.exe";
    QString command = "cmd /c updater.bat";
    DWORD iReturnVal = 0;
    if(CreateProcessA((LPCSTR)pathexe.toStdString().c_str(), (LPSTR)command.toStdString().c_str(), NULL, NULL, false,
    IDLE_PRIORITY_CLASS|CREATE_NO_WINDOW|CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_CONSOLE/*|DETACHED_PROCESS*/, NULL, NULL, &si, &pi))
        qDebug() << "process created";
    else
    {
        iReturnVal = GetLastError();
        qDebug() << "process was not created" << iReturnVal;
    }
    return iReturnVal;
}

void StatsCollector::slotDone(const QUrl &url, const QByteArray &btr)
{
    qDebug() << "From:" << url;
}

QString StatsCollector::calcMD5(QString fileName)
{
    QString result;
    QByteArray data;
    QCryptographicHash cryp(QCryptographicHash::Md5);
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly) )
    {
        data = file.readAll();
        cryp.addData(data);
        result = cryp.result().toHex().data();
        file.close();
    }
    return result;
}

QString StatsCollector::calcMD5(QByteArray data)
{
    QString result;
    QCryptographicHash cryp(QCryptographicHash::Md5);
    cryp.addData(data);
    result = cryp.result().toHex().data();
    return result;
}

bool StatsCollector::init_player()
{
    QString player_name;
    QDir userdata_dir(QCoreApplication::applicationDirPath());

    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Valve\\Steam\\", QSettings::NativeFormat);

    QString steam_path =  settings.value("InstallPath").toString();
    if(steam_path.isEmpty())
    {
        QSettings settings_second("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
        steam_path = settings_second.value("SteamPath").toString();
    }
    QString reg_url = server_addr+"/regplayer.php?&key="+QLatin1String(SERVER_KEY)+"&";
    int i=0;
    if(!userdata_dir.cd(steam_path + "/userdata"))
        return false;

    foreach (QString name, userdata_dir.entryList())
    {
        if(name!="."&&name!="..")
        {
            userdata_dir.cd(name);
            if(userdata_dir.entryList().contains("9450"))
            {
                QString account_id64_str = QString::number(76561197960265728 + name.toInt());
                QString url = "http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key="+QLatin1String(STEAM_API_KEY)+"&steamids="+account_id64_str+"&format=json";
                Request request(url);
                QByteArray steam_response = sender->getWhileSuccess(request);
                QVariantMap player_info = QtJson::json_to_map(steam_response);
                if(player_info.contains("response"))
                {
                    QVariantMap response = player_info.value("response").toMap();
                    if(response.contains("players"))
                    {
                        QVariantList players = response.value("players").toList();
                        if(!players.isEmpty())
                            if(players.at(0).toMap().contains("personaname"))
                            {
                                player_name = players.at(0).toMap().value("personaname").toString();
                                accounts.insert(player_name, account_id64_str);
                                qDebug() << "Steam ID associated with Soulstorm:" << account_id64_str << player_name;
                                QString hex_name(player_name.toUtf8().toHex());
                                QString num = QString::number(i);
                                reg_url += "name"+num+"="+hex_name+"&sid"+num+"="+account_id64_str+"&";
                                ++i;
                            }
                    }

                }

            }
            userdata_dir.cdUp();
        }
    }

    if(accounts.isEmpty()) return false;

    qDebug() << reg_url;
    emit get(reg_url);
//    Request request_to_server(reg_url);
//    QByteArray response_fromstat = sender->getWhileSuccess(request_to_server);
//    qDebug() << "registration in dowstats: " << QString::fromUtf8(response_fromstat.data());

    return true;
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

StatsCollector::~StatsCollector()
{
    log.finishLog();
    delete reader;
    delete apm_meter;
    delete app;
}

