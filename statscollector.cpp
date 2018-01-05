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
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <psapi.h>

using namespace tyti;

DWORD fog_target = 0x008282F0;
DWORD hpbars_target = 0x00956596;
BYTE CodeFragmentFT[6] = {0xD9, 0x81, 0x60, 0x0C, 0x00, 0x00};
BYTE fog_toggle[6] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
BYTE tempFT[6] = {0};
BYTE CodeFragmentHT[4] = {0x84, 0xDB, 0x74, 0x19};
BYTE hpbars_toggle[4] = {0x90, 0x90, 0x90, 0x90};
BYTE tempHT[4] = {0};

StatsCollector::StatsCollector(QObject *parent) :
    QObject(parent)
{
    haveModuleInfo = false;
    curFog = false;
    curHP = false;
    log.installLog("stats.log");
    server_addr = "http://www.dowstats.h1n.ru";
    cur_time = QDateTime::currentDateTime();
    reader = new GameInfoReader();
    lpSharedMemory = nullptr;
    hSharedMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TGameInfo), L"DXHook-Shared-Memory");
    if(hSharedMemory)
        lpSharedMemory = (PGameInfo)MapViewOfFile(hSharedMemory, FILE_MAP_WRITE, 0, 0, 0);
    else
        qDebug() << "CreateFileMapping: Error" << GetLastError();

    if(lpSharedMemory){
        lpSharedMemory->fontsInited = true;
        memset(lpSharedMemory->players, 0, 400);
        lpSharedMemory->playersNumber = 0;
        lpSharedMemory->downloadProgress = 0;
        memset(&lpSharedMemory->mapName[0], 0, 50);
        lpSharedMemory->CurrentAPM = 0;
        lpSharedMemory->AverageAPM = 0;
        lpSharedMemory->MaxAPM = 0;
        lpSharedMemory->showMenu = false;
        lpSharedMemory->showRaces = false;
        lpSharedMemory->showAPM = false;
        lpSharedMemory->sidsAddrLock = false;
    }
    else
        qDebug() << "MapViewOfFile: Error" << GetLastError();

    sender_thread = new QThread();
    sender = new RequestSender();
    sender->moveToThread(sender_thread);
    connect(sender_thread, SIGNAL(finished()), sender_thread, SLOT(deleteLater()));
    connect(sender_thread, SIGNAL(finished()), sender, SLOT(deleteLater()));
    connect(this, SIGNAL(POST_REQUEST(QString,QString,QString,QByteArray,QString)),
            sender, SLOT(POST_REQUEST(QString,QString,QString,QByteArray,QString)));
    connect(this, SIGNAL(GET_REQUEST(QString, QString)), sender, SLOT(GET_REQUEST(QString, QString)));

    sender_thread->start();

}

bool StatsCollector::start()
{
    QSettings settings("stats.ini", QSettings::IniFormat);
    // получаем из файла конфигураций данные о размере буфера
    version = settings.value("info/version", "0.0.0").toString().remove(".");
    server_addr = settings.value("info/serverAddr", "http://www.dowstats.h1n.ru/").toString();
    qDebug() << "Server:" << server_addr << "Version:" << version;
    enableStats = settings.value("settings/enableStats", true).toBool();
    enableDXHook = false;
    if(lpSharedMemory){
        lpSharedMemory->enableDXHook = enableDXHook = settings.value("settings/enableDXHook", false).toBool();
        if(enableDXHook){
            lpSharedMemory->showMenu = showMenu = settings.value("settings/showMenu", true).toBool();
            lpSharedMemory->showRaces = showRaces = settings.value("settings/showRaces", true).toBool();
            lpSharedMemory->showAPM = showAPM = settings.value("settings/showAPM", true).toBool();
            lpSharedMemory->version = version.toInt();
            lpSharedMemory->statsThrId = GetCurrentThreadId();
        }
    }
    if(!enableStats) qDebug() << "STATS DISABLED";
    if(enableDXHook){
        qDebug() << "Creating shortcuts";
        QString showMenuHK = settings.value("hotkeys/showMenuHK", "Ctrl+Shift+M").toString();
        QString showRaceHK = settings.value("hotkeys/showRaceHK", "Ctrl+Shift+R").toString();
        QString showAPMHK  = settings.value("hotkeys/showAPMHK", "Ctrl+Shift+A").toString();

        QxtGlobalShortcut* menuShortcut = new QxtGlobalShortcut(QKeySequence(showMenuHK), this);
        connect(menuShortcut, SIGNAL(activated()), this, SLOT(toggleMenuVisibility()));
        QxtGlobalShortcut* racesShortcut = new QxtGlobalShortcut(QKeySequence(showRaceHK), this);
        connect(racesShortcut, SIGNAL(activated()), this, SLOT(toggleRacesVisibility()));
        QxtGlobalShortcut* apmShortcut = new QxtGlobalShortcut(QKeySequence(showAPMHK), this);
        connect(apmShortcut, SIGNAL(activated()), this, SLOT(toggleAPMVisibility()));
    } else qDebug() << "DXHOOK DISABLED";

    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("http://www.google.com"));
    QNetworkReply *reply = nam.get(req);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    while(!reply->bytesAvailable()){
        qDebug() << "Player are not connected to the internet";
        Sleep(5000);
        QCoreApplication::processEvents();
        reply = nam.get(req);
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }
    qDebug() << "Player are connected to the internet";
    if(init_player())
        qDebug() << "Player initialization successfull";
    else{
        qDebug() << "Player initialization failed";
        return false;
    }

    ss_path = get_soulstorm_installlocation();
    qDebug() << "Soulstorm:" << ss_path+"\\Soulstorm.exe";
    reader->init(ss_path, sender_steamID, sender_name);

    apm_thread = new QThread;
    apm_meter.moveToThread(apm_thread);

    connect(apm_thread, SIGNAL(finished()), apm_thread, SLOT(deleteLater()));
    connect(this, SIGNAL(start_apm_meter()), &apm_meter, SLOT(start()));
    connect(&stats_timer, SIGNAL(timeout()), this, SLOT(check_game()));

    apm_thread->start();
    stats_timer.start(5000);

    send_logfiles();

    return true;
}

QString StatsCollector::GetRandomString() const
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   const int randomStringLength = 12; // assuming you want random strings of 12 characters

   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = qrand() % possibleCharacters.length();
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}

void StatsCollector::check_game()
{
    QSettings settings("stats.ini", QSettings::IniFormat);
    QDateTime last_modified = QFileInfo(reader->get_cur_profile_dir() +"/testStats.lua").lastModified();
    // если время последнего изменения файла больше предыдущего
    // то это либо начало игры, либо конец, что необходимо обработать
    if(last_modified>cur_time){
        QTimer showRacesTimer;
        bool procIsActive = true, isObserver = false;
        int e_code = 0;
        cur_time = QDateTime::currentDateTime();
        int game_state = reader->readySend(true);
        switch (game_state){
            // игра завершилась и она не реплей
            case 0:
                if(enableDXHook) lpSharedMemory->showMenu = showMenu;
                if(!enableStats)
                    break;
                qDebug() << "Getting statistics";
                GetSteamPlayersInfo(false);
                e_code = reader->read_game_info(&AllPlayersInfo, apm_meter.getTotalActions());
                if(e_code==0)
                {
                    QString url = server_addr+"/connect.php?"+reader->get_game_stats()+"key="+QLatin1String(SERVER_KEY)+"&version="+version+"&";
                    bool sendReplays = settings.value("settings/sendReplays", true).toBool();
                    if(sendReplays&&!reader->is_playback_error()){
                        qDebug() << "send stats with replay";
                        emit POST_REQUEST(url,
                                      reader->get_playback_name(),
                                      "application/octet-stream",
                                      reader->get_playback_file(),
                                      "stats"+GetRandomString());
                    }
                    else{
                        qDebug() << "send stats without replay";
                        emit GET_REQUEST(url, "stats"+GetRandomString());
                    }
                }
                else
                    reader->get_error_debug(e_code);

                break;
            // игра не завершилась
            case 1:
                qDebug() << "Game Start" << game_state;
                if(enableDXHook){
                    lpSharedMemory->showMenu = false;
                    if(showRaces){
                        lpSharedMemory->showRaces = true;
                        QStringList players = reader->get_players(true);
                        if(!players.isEmpty()){
                            for(int i=0; i<players.size(); ++i){
                                qDebug() << players.at(i);
                                memcpy(&lpSharedMemory->players[i][0], players.at(i).toStdString().data(), 100);
                            }

                            showRacesTimer.setSingleShot(true);
                            showRacesTimer.start(25000);
                            connect(&showRacesTimer, SIGNAL(timeout()), this, SLOT(toggleRacesVisibility()));
                        }else{
                            qDebug() << "Player is observer";
                            isObserver = true;
                        }
                    }
                    lpSharedMemory->showAPM = showAPM;
                }

                // если игра идет и счетчик apm не запущен, то запустим его
                if(!isObserver&&apm_meter.stopped)
                    emit start_apm_meter();
                GetSteamPlayersInfo(false);
                // перенесем запуск ридера сидов для того чтобы выполнить чтение позже
                // ждем пока закончится игра
                // здесь стоит так же проверять находится ли игра в списке процессов
                while(procIsActive&&game_state!=0&&game_state<3){
                    procIsActive = systemWin32::findProcessByWindowName("Dawn of War: Soulstorm");
                    if(!isObserver&&enableDXHook){
                        lpSharedMemory->CurrentAPM = apm_meter.getCurrentAPM();
                        lpSharedMemory->AverageAPM = apm_meter.getAverageAPM();
                        if(lpSharedMemory->MaxAPM>0||apm_meter.getTime()>=(DWORD)120000)
                            lpSharedMemory->MaxAPM = apm_meter.getMaxAPM();
                    }
                    Sleep(1000);
                    QCoreApplication::processEvents();
                    game_state = reader->readySend();
                }
                qDebug() << "Game Stop" << game_state << procIsActive;
                apm_meter.stop();
                if(enableDXHook){
                    for(int i=0; i<8; ++i)
                        memset(&lpSharedMemory->players[i][0], 0, 100);
                    lpSharedMemory->CurrentAPM = 0;
                    lpSharedMemory->AverageAPM = 0;
                    lpSharedMemory->MaxAPM = 0;
                }
                break;
            // игра - просмотр реплея
            case 2:
                qDebug() << "Game Start Playback";
                if(enableDXHook) lpSharedMemory->showMenu = false;
                break;

            default:
                qDebug() << "Stats Break";
                break;
        }
        send_logfiles();
    }else{
        if(systemWin32::findProcessByWindowName("Dawn of War: Soulstorm")){
            // получим новые значения флагов
            enableStats = settings.value("settings/enableStats", true).toBool();
            enableDXHook = settings.value("settings/enableDXHook", false).toBool();
            lpSharedMemory->showMenu  = showMenu  = settings.value("settings/showMenu", true).toBool();
            lpSharedMemory->showRaces = showRaces = settings.value("settings/showRaces", true).toBool();
            lpSharedMemory->showAPM   = showAPM   = settings.value("settings/showAPM", true).toBool();

            if(reader->is_game_restarted()){
                check_name();
                processFlags(true);
            }
            GetSteamPlayersInfo();
        }else{
            closeWithGame = settings.value("settings/closeWithGame", false).toBool();
            if(closeWithGame){
                qDebug() << "closeWithGame";
                QCoreApplication::quit();
            }
            AllPlayersInfo.clear();
            reader->reset();
        }
    }
}

void StatsCollector::updateProgress(qint64 bytesSent, qint64 bytesTotal)
{
    int progress = (int)(100*bytesSent/bytesTotal);
    if(enableDXHook){
        lpSharedMemory->downloadProgress = progress;
        if(progress==100){
            Sleep(1000);
            lpSharedMemory->downloadProgress = 0;
            Sleep(10000);
            memset(&lpSharedMemory->mapName[0], 0, 50);
            disconnect(sender, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
        }
    }
}

void StatsCollector::toggleAPMVisibility()
{
    if(lpSharedMemory){
        lpSharedMemory->showAPM = !lpSharedMemory->showAPM;
        qDebug() << "APMVisibility:" << (lpSharedMemory->showAPM?"On":"Off");
    }
}

void StatsCollector::exitHandler()
{
    qDebug() << "exitHandler()";
    QCoreApplication::quit();
}

void StatsCollector::toggleRacesVisibility()
{
    if(lpSharedMemory){
        lpSharedMemory->showRaces = !lpSharedMemory->showRaces;
        qDebug() << "RacesVisibility:" << (lpSharedMemory->showRaces?"On":"Off");
    }
}

void StatsCollector::toggleMenuVisibility()
{
    if(lpSharedMemory){
        lpSharedMemory->showMenu = !lpSharedMemory->showMenu;
        qDebug() << "MenuVisibility:" << (lpSharedMemory->showMenu?"On":"Off");
    }
}

void StatsCollector::download_map(QString map_name)
{
    qDebug() << "Downloading" << map_name;
    if(map_name.isEmpty())
        return;

    if(enableDXHook){
        memcpy(&lpSharedMemory->mapName[0], map_name.toStdString().data(), 50);
        connect(sender, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
    }

    QString url_str = server_addr+"/update.php?sid="+sender_steamID+"&key=" + QLatin1String(SERVER_KEY);
    url_str += "&filetype=map&name="+QUrl::toPercentEncoding(map_name,""," ")+".zip";
    emit GET_REQUEST(url_str, ss_path+"/DXP2/Data/Scenarios/mp/"+map_name+".zip");
}

bool StatsCollector::send_logfiles()
{
    if(sender_steamID.isEmpty()) return false;
    bool result = false;
    QDir cur_dir(ss_path);
    if(cur_dir.exists("stats_logs")) removeDir(ss_path+"/stats_logs");
    cur_dir.mkdir("stats_logs");
    QFile::copy(ss_path+"/update.log", ss_path+"/stats_logs/update.log");
    QFile::copy(ss_path+"/dxhook.log", ss_path+"/stats_logs/dxhook.log");
    QFile::copy(ss_path+"/stats.log", ss_path+"/stats_logs/stats.log");
    QFile::copy(ss_path+"/warnings.log", ss_path+"/stats_logs/warnings.log");
    QFile::copy(ss_path+"/stats.ini", ss_path+"/stats_logs/stats.ini");
    QFile::copy(reader->get_cur_profile_dir() +"/testStats.lua",ss_path+"/stats_logs/testStats.lua");

    if(!sender->compress(sender_steamID+".zip", ss_path+"/stats_logs", ""))
        return result;

    QFile log(sender_steamID+".zip");
    result = log.open(QIODevice::ReadOnly);
    if(result){
        QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+sender_steamID+"&version="+version+"&type=5";
        emit POST_REQUEST(url, sender_steamID+".zip", "text/txt", log.readAll(), "logs");
        log.close();
    }
    log.remove();
    removeDir(ss_path+"/stats_logs");

    return result;
}

bool StatsCollector::removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists()) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = QDir().rmdir(dirName);
    }
    return result;
}

void StatsCollector::processFlags(bool force)
{
    QSettings settings("stats.ini", QSettings::IniFormat);
    // получим новые значения флагов
    // для некоторых флагов нужно запоминать старые значения чтобы не выполнять
    // один и тот же код несколько раз, на случай если флаги не изменились
    bool tFog = settings.value("settings/disableFog", false).toBool();
    bool tHP = settings.value("settings/showHP", false).toBool();

    if(tFog==curFog&&tHP==curHP&&!force)
        return;

    HWND hWnd = FindWindow(NULL, L"Dawn of War: Soulstorm");
    DWORD PID;
    if(hWnd==NULL){
        return;
    }
    GetWindowThreadProcessId(hWnd, &PID);
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 0, PID);

    if(hProcess==nullptr){
        qDebug() << "Could not open process" << GetLastError();
        return;
    }
    if(tFog!=curFog||force){
        if(tFog)
        {
            qDebug() << "Disable fog";
            ReadProcessMemory(hProcess, (PVOID)fog_target, tempFT, 6, NULL);
            if(memcmp(tempFT, CodeFragmentFT, 6)==0)
                WriteProcessMemory(hProcess, (PVOID)fog_target, fog_toggle, 6, NULL);
        }
        else
        {
            qDebug() << "Enable fog";
            ReadProcessMemory(hProcess, (PVOID)fog_target, tempFT, 6, NULL);
            if(memcmp(tempFT, fog_toggle, 6)==0)
                WriteProcessMemory(hProcess, (PVOID)fog_target, CodeFragmentFT, 6, NULL);
        }
        curFog = tFog;
    }
    if(tHP!=curHP||force){
        if(tHP)
        {
            qDebug() << "Show hitpoint bars";
            ReadProcessMemory(hProcess, (PVOID)hpbars_target, tempHT, 4, NULL);
            if(memcmp(tempHT, CodeFragmentHT, 4)==0)
                WriteProcessMemory(hProcess, (PVOID)hpbars_target, hpbars_toggle, 4, NULL);
        }
        else
        {
            qDebug() << "Hide hitpoint bars";
            ReadProcessMemory(hProcess, (PVOID)hpbars_target, tempHT, 4, NULL);
            if(memcmp(tempHT, hpbars_toggle, 4)==0)
                WriteProcessMemory(hProcess, (PVOID)hpbars_target, CodeFragmentHT, 4, NULL);
        }
        curHP = tHP;
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
    qDebug() << "Steam:" << steam_path;

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

    QString player_name;

    Request request("http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key="+QLatin1String(STEAM_API_KEY)+"&steamids="+sender_steamID+"&format=json");
    QVariantMap player_info = QtJson::json_to_map(sender->get(request));
    QVariantMap response = player_info.value("response", QVariantMap()).toMap();
    QVariantList players = response.value("players", QVariantList()).toList();
    QVariantMap player = players.value(0, QVariantMap()).toMap();
    player_name = player.value("personaname", "").toString();
    if(!player_name.isEmpty())
        sender_name = player_name;

    // если игрок на данном аккаунте сейчас играет, и игра Soulstorm, то добавим его в список
    // после этого можно так же прерывать цикл, так как нужный игрок найден
    if(player.value("personastate", 0).toInt()==1&&
            player.value("gameid", 0).toInt()==9450)
        qDebug() << "Player" << player_name << "is online";
    else
        qDebug() << "Player" << player_name << "is offline";
    qDebug() << "Player's nickname and Steam ID associated with Soulstorm:" << sender_name << sender_steamID;

    register_player(sender_name, sender_steamID, true);
    return true;
}

void StatsCollector::check_name()
{
    QString player_name;
    Request request("http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key="+QLatin1String(STEAM_API_KEY)+"&steamids="+sender_steamID+"&format=json");
    QVariantMap player_info = QtJson::json_to_map(sender->get(request));
    QVariantMap response = player_info.value("response", QVariantMap()).toMap();
    QVariantList players = response.value("players", QVariantList()).toList();
    QVariantMap player = players.value(0, QVariantMap()).toMap();
    player_name = player.value("personaname", "").toString();
    if(sender_name!=player_name){
        sender_name = player_name;
        qDebug() << "Player nickname and Steam ID associated with Soulstorm:" << sender_name << sender_steamID;
        reader->setSender_name(sender_name);
        register_player(sender_name, sender_steamID);
    }
}

void StatsCollector::register_player(QString name, QString sid, bool init)
{
    QByteArray enc_name = QUrl::toPercentEncoding(name,""," ");
    QString reg_url = server_addr+"/regplayer.php?name="+enc_name+"&sid="+sid+"&version="+version+"&sender_sid="+sender_steamID+"&";
    if(init) reg_url += "init=true&";
    reg_url += "key="+QLatin1String(SERVER_KEY)+"&";
    emit GET_REQUEST(reg_url);
}

QString StatsCollector::get_soulstorm_installlocation()
{
    QString path = qApp->applicationDirPath();
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
    stats_timer.stop();
    apm_thread->quit();
    apm_thread->wait();
    sender_thread->quit();
    sender_thread->wait();

    if(lpSharedMemory){
        memset(lpSharedMemory, 0, sizeof(TGameInfo));
        UnmapViewOfFile(lpSharedMemory);
    }
    if(hSharedMemory){
        CloseHandle(hSharedMemory);
    }

    delete reader;
    log.finishLog();
}

int StatsCollector::GetSteamPlayersInfo(bool get_stats) {

//    addresses.clear();
    PlayersInfo.clear();
    HWND hWnd = FindWindow(NULL, L"Dawn of War: Soulstorm");
    DWORD PID;
    if(hWnd==NULL){
        return GetLastError();
    }
    GetWindowThreadProcessId(hWnd, &PID);

    // Получение дескриптора процесса
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                       PROCESS_VM_READ, FALSE, PID);
    if(hProcess==nullptr){
        qDebug() << "OpenProcess Error" << GetLastError();
        return GetLastError();
    }

    if(!haveModuleInfo){
        HANDLE hthSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID);
        if (hthSnapshot!=INVALID_HANDLE_VALUE)
        {
            MODULEENTRY32 me = { sizeof(me) };
            Module32First(hthSnapshot, &me);
            do moduleInfo.insert((DWORD)me.modBaseAddr, (DWORD)me.modBaseSize);
            while(Module32Next(hthSnapshot, &me));
            CloseHandle(hthSnapshot);
            haveModuleInfo = true;
        }
    }
    QByteArray buffer(30400, 0);
    for(int i=0; i<10; ++i){
        lpSharedMemory->sidsAddrLock = true;
//        for (PVOID readAddr = lpSharedMemory->sidsAddr[i]; readAddr < lpSharedMemory->sidsAddr[i]+0x23000; readAddr += buffer.size() - 200) {
            PVOID readAddr;
//            if(i>9)
//                readAddr = (PVOID)*(lpSharedMemory->sidsAddr[i-10]);
//            else
            readAddr = lpSharedMemory->sidsAddr[i];
            qDebug() << readAddr;
            SIZE_T bytesRead = 0;
            // если функция вернула не ноль, то продолжим цикл
            if(!ReadProcessMemory(hProcess, readAddr, (PVOID)buffer.data(), buffer.size(), &bytesRead))
                continue;
//            int zero_counter=0;
            for (uint i = 0; i < bytesRead - 200; i++) {
                bool match = false;
//                if(buffer[i]==0){
//                    ++zero_counter;
//                    if(zero_counter==16) break;
//                }else zero_counter=0;
                for (uint j = 0; j < sizeof(steamHeader); j++)
                    if (buffer[i + j] != steamHeader[j]){
                        match = false;
                        break;
                    }else match = true;
                if (!match)
                    continue;
                int nickPos = i + 56;
                if (buffer[nickPos] < 50 &&
                    buffer[nickPos] > 0 &&
                    buffer[nickPos + 1] == 0 &&
                    buffer[nickPos + 2] == 0 &&
                    buffer[nickPos + 3] == 0 &&
                    buffer[nickPos - 1] == 0 &&
                    buffer[nickPos - 2] == 0 &&
                    buffer[nickPos - 3] == 0 &&
                    buffer[nickPos - 4] == 0 &&
                    buffer[nickPos+4+buffer[nickPos]*2]   == 0 &&
                    buffer[nickPos+4+buffer[nickPos]*2+1] == 0 &&
                    buffer[nickPos+4+buffer[nickPos]*2+2] == 0 &&
                    buffer[nickPos+4+buffer[nickPos]*2+3] == 0) {
                    QString steamIdStr = QString::fromUtf16((ushort*)buffer.mid(i + 18, 34).data()).left(17);
                    if(!steamIdStr.contains(QRegExp("^[0-9]{17}$"))){
//                            qDebug() << mbi.BaseAddress << (PVOID)(readAddr+i) << (PVOID)mbi.RegionSize << steamIdStr << "false";
//                            qDebug() << (readAddr+i) << steamIdStr << "false";
                        continue;
                    }
                    if(get_stats&&!PlayersInfo.contains(steamIdStr)){
//                            PlayersInfo.insert(steamIdStr, nick);
//                            qDebug() << mbi.BaseAddress << (PVOID)(readAddr+i) << (PVOID)mbi.RegionSize << steamIdStr << "true";
//                            qDebug() << (readAddr+i) << steamIdStr << "true";
                        PlayersInfo.append(steamIdStr);
                    }
                    if(!AllPlayersInfo.contains(steamIdStr)){
                        QString nick = QString::fromUtf16((ushort*)buffer.mid(nickPos + 4, buffer[nickPos] * 2).data()).left(buffer[nickPos]);
                        AllPlayersInfo.insert(steamIdStr, nick);
                    }
                }
            }
//        }
    }
//    lpSharedMemory->sidsAddr.clear();
    lpSharedMemory->sidsAddrLock = false;
//    long ptr1Count = 0x00000000; // адресс после недоступной зоны
//    MEMORY_BASIC_INFORMATION mbi; // Объявляем структуру
//    LPCVOID ptr1 = (LPCVOID)ptr1Count;
//    while (ptr1Count < 0x7FFF0000) // До конца виртуальной памяти для данного процесса
//    {
//        qDebug() << ptr1Count << (ptr1Count <= 0x7FFF0000);
//        if(moduleInfo.keys().contains(ptr1Count)){
//            qDebug() << (PVOID)ptr1Count << (PVOID)moduleInfo.value(ptr1Count);
//            ptr1Count = ptr1Count + moduleInfo.value(ptr1Count);
//            ptr1 = (PVOID)ptr1Count;
//            continue;
//        }
//        VirtualQueryEx(hProcess, ptr1, &mbi, sizeof(mbi));
//        if(mbi.Type==MEM_PRIVATE&&
//            mbi.AllocationProtect==PAGE_READWRITE&&
//            mbi.Protect==PAGE_READWRITE&&
//            mbi.State==MEM_COMMIT&&mbi.RegionSize<0xFFF000)
//        {
//    while (ptr1Count < 0x7FFF0000) // До конца виртуальной памяти для данного процесса
//    {
//        qDebug() << ptr1Count << (ptr1Count <= 0x7FFF0000);
//        if(moduleInfo.keys().contains(ptr1Count)){
//            qDebug() << (PVOID)ptr1Count << (PVOID)moduleInfo.value(ptr1Count);
//            ptr1Count = ptr1Count + moduleInfo.value(ptr1Count);
//            ptr1 = (PVOID)ptr1Count;
//            continue;
//        }

//        VirtualQueryEx(hProcess, ptr1, &mbi, sizeof(mbi));
//        if(mbi.Type==MEM_PRIVATE&&
//            mbi.AllocationProtect==PAGE_READWRITE&&
//            mbi.Protect==PAGE_READWRITE&&
//            mbi.State==MEM_COMMIT&&mbi.RegionSize<0xFFF000)
//        {
//            for (PCHAR readAddr = (PCHAR)mbi.BaseAddress; readAddr < (PCHAR)mbi.BaseAddress+mbi.RegionSize; readAddr += buffer.size() - 200) {
//                SIZE_T bytesRead = 0;
//                if(!ReadProcessMemory(hProcess, (PVOID)readAddr, (PVOID)buffer.data(), buffer.size(), &bytesRead))
//                    continue;
//                for (uint i = 0; i < bytesRead - 200; i++) {
//                    bool match = false;
//                    for (uint j = 0; j < sizeof(steamHeader); j++)
//                        if (buffer[i + j] != steamHeader[j]){
//                            match = false;
//                            break;
//                        }else match = true;
//                    if (!match)continue;
//                    int nickPos = i + 56;
//                    if (buffer[nickPos] < 50 &&
//                        buffer[nickPos] > 0 &&
//                        buffer[nickPos + 1] == 0 &&
//                        buffer[nickPos + 2] == 0 &&
//                        buffer[nickPos + 3] == 0 &&
//                        buffer[nickPos - 1] == 0 &&
//                        buffer[nickPos - 2] == 0 &&
//                        buffer[nickPos - 3] == 0 &&
//                        buffer[nickPos - 4] == 0 &&
//                        buffer[nickPos+4+buffer[nickPos]*2]   == 0 &&
//                        buffer[nickPos+4+buffer[nickPos]*2+1] == 0 &&
//                        buffer[nickPos+4+buffer[nickPos]*2+2] == 0 &&
//                        buffer[nickPos+4+buffer[nickPos]*2+3] == 0) {
//                        QString steamIdStr = QString::fromUtf16((ushort*)buffer.mid(i + 18, 34).data()).left(17);
//                        if(!steamIdStr.contains(QRegExp("^[0-9]{17}$"))){
//                            qDebug() << (PVOID)(readAddr+i) << steamIdStr << "false";
//                            continue;
//                        }
//                        if(get_stats&&!PlayersInfo.contains(steamIdStr)){
//                            PlayersInfo.append(steamIdStr);
//                            qDebug() << (PVOID)(readAddr+i);
//                        }
//                        if(!AllPlayersInfo.contains(steamIdStr)){
//                            QString nick = QString::fromUtf16((ushort*)buffer.mid(nickPos + 4, buffer[nickPos] * 2).data()).left(buffer[nickPos]);
//                            AllPlayersInfo.insert(steamIdStr, nick);
//                        }
//                    }
//                }
//            }
//        }
//        ptr1Count = ptr1Count + (int)mbi.RegionSize;
//        ptr1 = (PVOID)ptr1Count;
//    }

//    qSort(addresses);
//    foreach(LPVOID addr, addresses)
//        qDebug() << addr;
//    foreach(LPVOID addr, addresses_finded)
//        qDebug() << "addresses_finded" << addr;

    if(!PlayersInfo.isEmpty()&&get_stats){
//        qDebug() << PlayersInfo;
        if(!PlayersInfo.contains(sender_steamID))
            PlayersInfo.append(sender_steamID);
        QString sids = PlayersInfo.join(",");
        Request test_request{server_addr+"/stats.php?key="+QLatin1String(SERVER_KEY)+"&sids="+sids+"&version="+version+"&sender_sid="+sender_steamID+"&"};
//        qDebug() << test_request.address();
        QByteArray reply = sender->get(test_request);

        if(enableDXHook){
            QVariantList players_info = QtJson::json_to_list(reply);
            int i = 0;
            foreach(QVariant item, players_info){
                QVariantMap tempMap = item.toMap();
                QByteArray stats_btr = tempMap.value("name", "").toString().toUtf8();
                if(!stats_btr.isEmpty()){
                    memset(lpSharedMemory->lobbyPlayers[i].name, 0, 100);
                    memcpy(lpSharedMemory->lobbyPlayers[i].name, stats_btr.data(), stats_btr.size());
                    lpSharedMemory->lobbyPlayers[i].race = tempMap.value("race", 0).toInt();
                    lpSharedMemory->lobbyPlayers[i].gamesCount = tempMap.value("gamesCount", 0).toInt();
                    lpSharedMemory->lobbyPlayers[i].winsCount = tempMap.value("winsCount", 0).toInt();
                    lpSharedMemory->lobbyPlayers[i].winRate = tempMap.value("winRate", 0).toInt();
                    lpSharedMemory->lobbyPlayers[i].mmr = tempMap.value("mmr", 0).toInt();
                    lpSharedMemory->lobbyPlayers[i].mmr1v1 = tempMap.value("mmr1v1", 0).toInt();
                    lpSharedMemory->lobbyPlayers[i].apm = tempMap.value("apm", 0).toDouble();
                    ++i;
                }
            }
            if(i)lpSharedMemory->playersNumber = i;
        }
    }
    CloseHandle(hProcess);
    return 0;
}
