#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QObject>
#include <QProcess>
#include <fstream>
#include <string>
#include <iostream>
#include <QCryptographicHash>
#include "statscollector.h"
#include "systemwin32.h"
#include "vdf_parser.hpp"
#include "OSDaB-Zip/unzip.h"
#include "monitor.h"

using namespace tyti;

BYTE CodeFragmentFT[6] = { 0, 0, 0, 0, 0 };
BYTE CodeFragmentHT[4] = { 0, 0, 0, 0 };
DWORD fog_target = 0x008282F0;
DWORD hpbars_target = 0x00956596;
BYTE fog_toggle[6] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
BYTE hpbars_toggle[4] = {0x90, 0x90, 0x90, 0x90};
//    DWORD dwOldProtectFT = 0;
//    DWORD dwOldProtectHT = 0;

StatsCollector::StatsCollector(QObject* pobj /*=0*/)
    :QObject(pobj)
{
    curFog = false;
    curHP = false;

    log.installLog();
    server_addr = "http://www.dowstats.h1n.ru";

    reader = new GameInfoReader();

    hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TGameInfo), L"DXHook-Shared-Memory");
    lpSharedMemory = (PGameInfo)MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);
    memset(lpSharedMemory, 0, sizeof(TGameInfo));

    sender_thread = new QThread( );
    sender = new RequestSender();
    sender->moveToThread(sender_thread);
    connect(sender_thread, SIGNAL(finished()), sender_thread, SLOT(deleteLater()));
    connect(sender_thread, SIGNAL(finished()), sender, SLOT(deleteLater()));
    connect(this, SIGNAL(POST_REQUEST(QString,QString,QString,QByteArray)),
            sender, SLOT(POST_REQUEST(QString,QString,QString,QByteArray)));
    connect(this, SIGNAL(GET_REQUEST(QString, QString)), sender, SLOT(GET_REQUEST(QString, QString)));

    sender_thread->start();

}

bool StatsCollector::start()
{

    QSettings settings(qApp->applicationDirPath()+"/stats.ini", QSettings::IniFormat);
    // получаем из файла конфигураций данные о размере буфера
    version = settings.value("info/version", "0.0.0").toString();
    server_addr = settings.value("settings/serverAddr", "http://www.dowstats.h1n.ru/").toString();
    bool mapPackDownloaded = settings.value("info/mapPackDownloaded", false).toBool();
    enableDXHook = settings.value("settings/enableDXHook", false).toBool();
    enableStats = settings.value("settings/enableStats", true).toBool();

    Request request(server_addr + "/update.php?key=" + QLatin1String(SERVER_KEY) + "&");
    QString global_version = QString::fromUtf8(sender->getWhileSuccess(request).data());
    global_version = global_version.insert(1, '.').insert(3, '.').remove('\n').remove('\r');

    qDebug() << "server address:" << server_addr;
    qDebug() << "stats version:" << version;
    qDebug() << "global version:" << global_version;

    QProcess updater_proc;
    if(updater_proc.startDetached("SSStatsUpdater.exe"))
    {
        qDebug() << "SSStatsUpdater started successfully";
        if(version.remove(".").toInt()<global_version.remove(".").toInt())
            return false;
    }
    else
        qDebug() << "SSStatsUpdater failed to start";


    QProcess ssstats;
    if(ssstats.startDetached("SSStats.exe"))
        qDebug() << "Stats started successfully";
    else
        qDebug() << "Stats failed to start";

    qDebug() << "Player initialization";
    if(!init_player())
    {
        qDebug() << "Player initialization failed";
        return false;
    }

    ss_path = get_soulstorm_installlocation();
    reader->init(ss_path, sender_steamID, sender_name);

    apm_thread = new QThread;
    monitor_thread = new QThread;
    monitor.moveToThread(monitor_thread);
    apm_meter.moveToThread(apm_thread);

    QObject::connect(apm_thread, SIGNAL(finished()), apm_thread, SLOT(deleteLater()));
    QObject::connect(monitor_thread, SIGNAL(finished()), monitor_thread, SLOT(deleteLater()));

    QObject::connect(this, SIGNAL(start_apm_meter()), &apm_meter, SLOT(start()));
    QObject::connect(this, SIGNAL(start_monitor()), &monitor, SLOT(GetSteamPlayersInfo()));

    apm_thread->start();
    monitor_thread->start();


    if(!mapPackDownloaded)
    {
        QString filename = "SSStatsMapPack.zip";
        QString url_str = server_addr+"/update.php?key=" + QLatin1String(SERVER_KEY);
        url_str += "&filetype=map&name="+QUrl::toPercentEncoding(filename.section(".",0,0),""," ")+"."+filename.section(".",1,1);
        emit GET_REQUEST(url_str, ss_path+"/"+filename);
        settings.setValue("info/mapPackDownloaded", 1);
        settings.sync();
    }

    qDebug() << "start";
    cur_time = QDateTime::currentDateTime();
    if(!enableStats) qDebug() << "STATS DISABLED";

    connect(&stats_timer, SIGNAL(timeout()), this, SLOT(check_game()));

    // отправим лог файлы
    send_logfile();

    stats_timer.start(5000);

    return true;
}

void StatsCollector::check_game()
{
    QSettings settings("stats.ini", QSettings::IniFormat);
    // получим новые значения флагов
    enableStats = settings.value("settings/enableStats", true).toBool();

    int e_code = 0;
    reader->reset();

    QDateTime last_modified = QFileInfo(reader->get_cur_profile_dir() +"/testStats.lua").lastModified();
    // если время последнего изменения файла больше предыдущего
    // то это либо начало игры, либо конец, что необходимо обработать
    if(enableStats&&(last_modified>cur_time)){
        cur_time = QDateTime::currentDateTime();
        int p_num=0, time=0;
        switch (reader->readySend()){
            // игра завершилась и она не реплей
            case 0:
                qDebug() << "Game Stop";

                e_code = reader->read_game_info(&monitor.PlayersInfo, apm_meter.getTotalActions());
                if(e_code==0)
                {
                    // добавим ключ
                    QString server_key  = "key=" + QLatin1String(SERVER_KEY) + "&";
                    bool sendReplays = settings.value("settings/sendReplays", true).toBool();
                    if(sendReplays){
                        qDebug() << "send stats with replay";
                        emit POST_REQUEST(server_addr+"/connect.php?"+reader->get_game_stats()+server_key,
                                      reader->get_playback_name(),
                                      "application/octet-stream",
                                      reader->get_playback_file());
                    }
                    else{
                        qDebug() << "send stats without replay";
                        emit GET_REQUEST(server_addr+"/connect.php?"+reader->get_game_stats()+server_key);
                    }
                }
                else
                    reader->get_error_debug(e_code);

                // отправим лог файлы
                send_logfile();

                break;
            // игра не завершилась
            case 1:
                emit start_monitor();

                qDebug() << "Game Start";

                if(enableDXHook)
                    foreach (QString player, reader->get_players()) {
                        if(!player.isEmpty()){
                            qDebug() << player;
                            memcpy(&lpSharedMemory->players[p_num][0], player.toStdString().data(), 100);
                        }
                        p_num++;
                    }
                // если игроки были веведены, то ждем 10 секунд,
                // чтобы игрок ознакомился с информацией и очищаем
                if(p_num&&enableDXHook)
                {
                    Sleep(20000);
                    memset(lpSharedMemory, 0, sizeof(TGameInfo));
                }

                // если игра идет и счетчик apm не запущен, то запустим его
                if(apm_meter.stopped)
                    emit start_apm_meter();

                // ждем пока закончится игра
                while(reader->readySend()!=0){
                    if(enableDXHook){
                        lpSharedMemory->CurrentAPM = apm_meter.getCurrentAPM();
                        lpSharedMemory->AverageAPM = apm_meter.getAverageAPM();
                        if(time>=120)
                            lpSharedMemory->MaxAPM = apm_meter.getMaxAPM();
                    }
                    Sleep(1000);
                    time++;
                }
                apm_meter.stop();
                if(enableDXHook)
                    memset(lpSharedMemory, 0, sizeof(TGameInfo));

                while(!monitor.finished)
                    Sleep(1000);

                break;
            // игра - просмотр реплея
            case 2:
                qDebug() << "Current game is playback";
                apm_meter.stop();
                break;
            default:
                break;
        }
    }
    else
    {
        // проверим валидность карты
        if(!reader->is_map_valid())
            download_map(reader->get_last_invalid_map(), enableDXHook);

        processFlags();
    }
}

void StatsCollector::download_map(QString map_name, bool enableDXHook)
{
    qDebug() << "Downloading" << map_name;
    if(map_name.isEmpty())
        return;
    QString url_str = server_addr+"/update.php?key=" + QLatin1String(SERVER_KEY);
    url_str += "&filetype=map&name="+QUrl::toPercentEncoding(map_name,""," ")+".zip";
    Request request;
    request.setAddress(url_str);
    // устанавливает переменную, в котой будет храниться
    // прогресс загрузки
    if(enableDXHook)
        memcpy(&lpSharedMemory->mapName[0], map_name.toStdString().data(), 50);
    connect(sender, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
    QByteArray btar = sender->get(request);
    if(btar.isEmpty())
    {
        qDebug() << "reply is empty";
        return;
    }
    QFile cur_file("temp.zip");
    if(!cur_file.open(QIODevice::WriteOnly))
    {
        qDebug() << "could not open file temp.zip";
        return;
    }
    cur_file.write(btar);
    cur_file.close();

    if(sender->decompress("temp.zip",ss_path+"/DXP2/Data/Scenarios/mp",""))
        qDebug() << "Download completed successfully";
    QFile::remove("temp.zip");
    if(enableDXHook){
        Sleep(1000);
        lpSharedMemory->downloadProgress = 0;
        Sleep(10000);
        memset(lpSharedMemory, 0, sizeof(TGameInfo));
    }
}
bool StatsCollector::send_logfile()
{
    QStringList files;
    files << "stats.log"
         << "warnings.log";
//         << "update.log"
//         << "dxhook.log";

    if(sender_steamID.isEmpty()) return false;
    bool result = false;
    for(int i=0; i<files.size(); ++i)
    {
        if(i==2) continue;
        QFile log(files.at(i));
        result = log.open(QIODevice::ReadOnly);
        if(result)
        {
            QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+sender_steamID+"&version="+version.remove(".")+"&type="+QString::number(i);
            emit POST_REQUEST(url, sender_steamID+".log", "text/txt", log.readAll());
            log.close();
        }
    }
    return result;
}


void StatsCollector::processFlags()
{
    QSettings settings("stats.ini", QSettings::IniFormat);
    // получим новые значения флагов
    // для некоторых флагов нужно запоминать старые значения чтобы не выполнять
    // один и тот же код несколько раз, на случай если флаги не изменились
    bool tFog = settings.value("settings/disableFog", false).toBool();
    bool tHP = settings.value("settings/showHP", false).toBool();

    if(tFog==curFog&&tHP==curHP)
        return;

    HWND hWnd = FindWindow(NULL, L"Dawn of War: Soulstorm");
    DWORD PID;
    if(hWnd==NULL){
        return;
    }
    GetWindowThreadProcessId(hWnd, &PID);
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 0, PID);

    if(hProcess==nullptr){
        qDebug() << "could not open process" << GetLastError();
        return;
    }
    if(tFog!=curFog){
        if(tFog)
        {
            ReadProcessMemory(hProcess, (PVOID)fog_target, CodeFragmentFT, 6, NULL);
            WriteProcessMemory(hProcess, (PVOID)fog_target, fog_toggle, 6, NULL);
        }
        else
            WriteProcessMemory(hProcess, (PVOID)fog_target, CodeFragmentFT, 6, NULL);
    }
    if(tHP!=curHP){
        if(tHP)
        {
            ReadProcessMemory(hProcess, (PVOID)hpbars_target, CodeFragmentHT, 4, NULL);
            WriteProcessMemory(hProcess, (PVOID)hpbars_target, hpbars_toggle, 4, NULL);
        }
        else
            WriteProcessMemory(hProcess, (PVOID)hpbars_target, CodeFragmentHT, 4, NULL);
    }
    CloseHandle(hProcess);

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
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Valve\\Steam\\", QSettings::NativeFormat);

    QString steam_path =  settings.value("InstallPath").toString();
    if(steam_path.isEmpty())
    {
        QSettings settings_second("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
        steam_path = settings_second.value("SteamPath").toString();
    }
    QString reg_url = server_addr+"/regplayer.php?";
    qDebug() << steam_path;

    int Timestamp = 0;
    std::ifstream file(QString(steam_path+"/config/loginusers.vdf").toStdString());
    vdf::object root = vdf::read(file);
    for (auto it : root.childs)
    {
        int temp = QString::fromStdString(it.second->attribs["Timestamp"]).toInt();
        if(temp>Timestamp)
        {
            sender_name = QString::fromStdString(it.second->attribs["PersonaName"]);
            sender_steamID = QString::fromStdString(it.first);
            Timestamp = temp;
        }
    }

    QString player_name, url = "http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key="+QLatin1String(STEAM_API_KEY)+"&steamids="+sender_steamID+"&format=json";
    Request request(url);
    QByteArray steam_response = sender->get(request);
    QVariantMap player_info = QtJson::json_to_map(steam_response);
    if(player_info.contains("response"))
    {
        QVariantMap response = player_info.value("response").toMap();
        if(response.contains("players"))
        {
            QVariantList players = response.value("players").toList();
            if(!players.isEmpty())
            {
                if(players.at(0).toMap().contains("personaname"))
                {
                    player_name = players.at(0).toMap().value("personaname").toString();
                    // если игрок на данном аккаунте сейчас играет, и игра Soulstorm, то добавим его в список
                    // после этого можно так же прерывать цикл, так как нужный игрок найден
                    if(players.at(0).toMap().value("personastate").toInt()==1&&
                            players.at(0).toMap().value("gameid").toInt()==9450)
                        qDebug() << "Player" << player_name << "is online";
                    else
                        qDebug() << "Player" << player_name << "is offline";
                }
                else
                    qDebug() << 5 << steam_response;
            }
            else
                qDebug() << 4 << steam_response;
        }
        else
            qDebug() << 3 << steam_response;
    }
    else
        qDebug() << 2 << steam_response;


    qDebug() << "Player nickname and Steam ID associated with Soulstorm:" << sender_name << sender_steamID;
    if(!player_name.isEmpty())
        sender_name = player_name;
    QString hex_name(sender_name.toUtf8().toHex());
    reg_url += "name="+hex_name+"&sid="+sender_steamID+"&ver="+version.remove(".")+"&";

    qDebug() << reg_url;
    reg_url += "key="+QLatin1String(SERVER_KEY)+"&";
    emit GET_REQUEST(reg_url);
    return true;
}

QString StatsCollector::get_soulstorm_installlocation()
{
    QString path = qApp->applicationDirPath();
    qDebug() << path+"/Soulstorm.exe";
    if(QFile::exists(path+"/Soulstorm.exe"))
        return path;
    QSettings thq("HKEY_LOCAL_MACHINE\\SOFTWARE\\THQ\\Dawn of War - Soulstorm", QSettings::NativeFormat);
    path = thq.value("installlocation", "").toString();
    if(path.isEmpty())
    {
        QSettings sega("HKEY_LOCAL_MACHINE\\SOFTWARE\\SEGA\\Dawn of War - Soulstorm", QSettings::NativeFormat);
        path = sega.value("installlocation", "").toString();
    }
    return path;
}

StatsCollector::~StatsCollector()
{
    qDebug() << "apm_meter thread stop";
    apm_thread->quit();
    apm_thread->wait();
    qDebug() << "monitor_thread thread stop";
    monitor_thread->quit();
    monitor_thread->wait();
    qDebug() << "sender thread stop";
    sender_thread->quit();
    sender_thread->wait();
    qDebug() << "UnmapViewOfFile";
    UnmapViewOfFile(lpSharedMemory);
    qDebug() << "CloseHandle";
    CloseHandle(hSharedMemory);
    qDebug() << "delete reader";
    delete reader;
    qDebug() << "StatsCollector destructor";
    log.finishLog();
}
