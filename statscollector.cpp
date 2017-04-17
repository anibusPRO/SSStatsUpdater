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
#include "statscollector.h"
#include "systemwin32.h"
#include "vdf_parser.hpp"
#include "OSDaB-Zip/unzip.h"

using namespace tyti;

StatsCollector::StatsCollector(QObject* pobj /*=0*/)
    :QObject(pobj)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

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
    connect(this, SIGNAL(post(QString,QString,QString,QByteArray)),
            sender, SLOT(post(QString,QString,QString,QByteArray)));
    connect(this, SIGNAL(get(QString)), sender, SLOT(get(QString)));
    sender_thread->start();

//    connect(sender, SIGNAL(done(const QUrl&, const QByteArray&)),
//    this, SLOT(slotDone(const QUrl&, const QByteArray&)));
}

void StatsCollector::start()
{

    // файл конфигураций
    QSettings settings("stats.ini", QSettings::IniFormat);

    // получаем из файла конфигураций данные о размере буфера
    version = settings.value("info/version", "0.0.0").toString();
    server_addr = settings.value("settings/serverAddr", "http://www.dowstats.h1n.ru/").toString();
    bool fog = settings.value("settings/disableFog", false).toBool();
    bool showHP = settings.value("settings/showHP", false).toBool();
    bool enableUpdates = settings.value("settings/enableUpdates", true).toBool();

    qDebug() << "stats version:" << version;
    qDebug() << "server address:" << server_addr;

    if(enableUpdates){
    QString filename="SSStatsUpdater.dll";
    QString url = server_addr + "/update.php?key=" + QLatin1String(SERVER_KEY);
    Request request(url + "&name=ssstats.md5&");
    QByteArray btar = sender->get(request);
    QString md5 = QString::fromUtf8(btar.data());
    qDebug() << "SSStatsUpdater.dll md5 checking...";
    if(!md5.contains(calcMD5(filename))){
        qDebug() << "wrong md5, updating...";
        // код который получает программу для автообновления, создает bat файл и запускает процесс, его выполняющий
        // bat файл удаляет старую программу для автообновления, а затем переименовывает скачанную новую
        request.setAddress(url+"&name="+filename+"&");
        bool success = true;
        QByteArray btar = sender->getWhileSuccess(request);
        qDebug() << filename << "successfully downloaded";
        filename = "SSStatsUpdaterTemp.dll";
        QFile cur_file(filename);
        if(cur_file.open(QIODevice::WriteOnly)){
            cur_file.write(btar);
            cur_file.close();
        }
        else
            success = false;

        if(success&&md5.contains(calcMD5(filename)))
            updateUpdater();
        else
            cur_file.remove();
    }
    else
        qDebug() << "SSStatsUpdater.dll md5 is ok";
    }
    qDebug() << "Player initialization";
    if(!init_player())
    {
        qDebug() << "Player initialization failed";
        return;
    }

    ss_path = get_soulstorm_installlocation();
    reader->set_ss_path(ss_path);
    reader->set_account(sender_steamID, sender_name);
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

    bool enableStats = settings.value("settings/enableStats", true).toBool();
    if(!enableStats)
        qDebug() << "STATS DISABLED";
    systemWin32 processesInWin;

    while(!stop){
        if(enableStats){
            TSinfo.refresh();
            last_time = TSinfo.lastModified();
        }
        // если время последнего изменения файла больше предыдущего
        if((last_time>old_time)){
            int p_num=0, time=0;
            switch (reader->readySend()){
                // если рузультат вызова 0, то это означает что игра завершилась и она не реплей
                case 0:
                    apm_meter->stop();
                    qDebug() << "Game Stop";

                    reader->setTotalActions(apm_meter->getTotalActions());
                    params = reader->get_game_info(path_to_profile);

                    if(!params.isEmpty()){
                        bool sendReplays = settings.value("settings/sendReplays", true).toBool();
                        if(sendReplays){
                            qDebug() << "send stats with reply";
                            emit post(server_addr+"/connect.php?"+params,
                                          reader->get_playback_name(),
                                          "application/octet-stream",
                                          reader->get_playback_file());
                        }
                        else{
                            qDebug() << "send stats without reply";
                            emit get(server_addr+"/connect.php?"+params);
                        }
                    }
                    else
                        qDebug() << "params is empty";

                    break;
                // игра не завершилась
                case 1:
                    // перед запуском игры проверяем включена ли отправка статстики
                    // если игрок проигрывает, то отключить статстику он уже не может, так как
                    // программа не заходит в этот участок кода, но перед запуском игры
                    // игрок может отключить статистику в любое время
                    enableStats = settings.value("settings/enableStats", true).toBool();
                    qDebug() << "Game Start";
                    // если игра идет и счетчик apm не запущен, то запустим его
                    if(apm_meter->stop_flag)
                        emit start_meter();
                    foreach (QString player, reader->get_players(path_to_profile)) {
                        if(!player.isEmpty()){
                            qDebug() << player;
                            memcpy(&lpSharedMemory->players[p_num][0], player.toStdString().data(), 100);
                        }
                        p_num++;
                    }
                    // если игроки были веведены, то ждем 10 секунд,
                    // чтобы игрок ознакомился с информацией и очищаем
                    if(p_num)
                    {
                        Sleep(20000);
                        memset(lpSharedMemory, 0, sizeof(TGameInfo));
                    }
                    // ждем пока закончится игра
                    while(reader->readySend()!=0&&processesInWin.findProcess("Soulstorm")){
                        lpSharedMemory->CurrentAPM = apm_meter->getCurrentAPM();
                        lpSharedMemory->AverageAPM = apm_meter->getAverageAPM();
                        if(time>=120)
                            lpSharedMemory->MaxAPM = apm_meter->getMaxAPM();
                        Sleep(1000);
                        time++;
                        processesInWin.updateProcessList();
                    }
                    memset(lpSharedMemory, 0, sizeof(TGameInfo));
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
            bool checkSS = settings.value("settings/checkSS", true).toBool();
            processesInWin.updateProcessList();
            // проверим есть ли процесс игры в списке запущенных
            // если нет, то завершим работу программы
            if(checkSS&&!processesInWin.findProcess("Soulstorm"))
            {
                qDebug() << "process not found";
                stop = true;
            }

            // проверим валидность карты
            if(!reader->is_map_valid())
                download_map(reader->get_last_invalid_map());

            // проверим не изменил ли игрок профиль
            cur_profile = reader->get_cur_profile_dir();
            if((!cur_profile.isNull())&&path_to_profile!=cur_profile){
                path_to_profile = cur_profile;
                TSinfo.setFile(path_to_profile + "/testStats.lua");
                old_time = TSinfo.lastModified();
                qDebug() << "path_to_profile:" << path_to_profile;
            }

            // отправим лог файлы
            send_logfile();
            Sleep(5000);

            // отключим туман и включим показ хп
            if(fog&&disableFog(showHP)){
                qDebug() << "fog disabled";
                fog = false;
            }
        }
    }
    if(thread->isRunning())
    {
        apm_meter->stop();
        thread->exit(0);
    }
    if(sender_thread->isRunning())
        sender_thread->exit(0);
}

void StatsCollector::download_map(QString map_name)
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

    if(decompress("temp.zip",ss_path+"/DXP2/Data/Scenarios/mp",""))
        qDebug() << "Download completed successfully";
    QFile::remove("temp.zip");
    Sleep(1000);
    lpSharedMemory->downloadProgress = 0;
    Sleep(10000);
    memset(lpSharedMemory, 0, sizeof(TGameInfo));

}

bool StatsCollector::send_logfile()
{
    QStringList files;
    files << "stats.log" <<
             "warnings.log";
    QString steamid = reader->get_steam_id();

    if(steamid.isEmpty()) return false;
    bool result;
    for(int i=0; i<files.size(); ++i)
    {
        QFile log(files.at(i));
        result = log.open(QIODevice::ReadOnly);
        if(result)
        {
            QString url = server_addr+"/logger.php?key="+QLatin1String(SERVER_KEY)+"&steamid="+steamid+"&version="+version.remove(".")+"&type="+QString::number(i);
            emit post(url, sender_steamID+".log", "text/txt", log.readAll());
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

bool StatsCollector::disableFog(bool showHP)
{
    bool success = false;
    DWORD target = 0x008282F0;
    DWORD hpbars_target = 0x00956596;
    BYTE fog_toggle[6] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    BYTE hpbars_toggle[6] = {0x90, 0x90, 0x90, 0x90};

    HWND hWnd = FindWindow(NULL, L"Dawn of War: Soulstorm");
    DWORD PID;
    GetWindowThreadProcessId(hWnd, &PID);
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 0, PID);

    if(hProcess)
    {
        success = WriteProcessMemory(hProcess, (PVOID)target, fog_toggle, 6, NULL)!=0;
        if(showHP)
            WriteProcessMemory(hProcess, (PVOID)hpbars_target, hpbars_toggle, 4, NULL);

        CloseHandle(hProcess);
    }
    else
        qDebug() << "could not open process";

//    qDebug() << hWnd << PID << hProcess;
//    DWORD errorMessageID = GetLastError();
//    qDebug() << errorMessageID;
//    LPSTR messageBuffer = nullptr;
//    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
//    std::string message(messageBuffer, size);
//    qDebug() << QString::fromStdString(message);
    //Free the buffer.
//    LocalFree(messageBuffer);

    return success;
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
    emit get(reg_url);
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
    UnmapViewOfFile(lpSharedMemory);
    CloseHandle(hSharedMemory);
    log.finishLog();
    delete reader;
    delete apm_meter;
}

bool StatsCollector::decompress(const QString& file, const QString& out, const QString& pwd)
{
    if (!QFile::exists(file))
    {
        qDebug() << "File does not exist.";
        return false;
    }

    UnZip::ErrorCode ec;
    UnZip uz;

    if (!pwd.isEmpty())
        uz.setPassword(pwd);

    ec = uz.openArchive(file);
    if (ec != UnZip::Ok)
    {
        qDebug() << "Failed to open archive: " << uz.formatError(ec).toLatin1().data();
        return false;
    }

    ec = uz.extractAll(out);
    if (ec != UnZip::Ok)
    {
        qDebug() << "Extraction failed: " << uz.formatError(ec).toLatin1().data();
        uz.closeArchive();
        return false;
    }

    return true;
}

void StatsCollector::updateProgress(qint64 bytesSent, qint64 bytesTotal)
{
    lpSharedMemory->downloadProgress = (int)(100*bytesSent/bytesTotal);
//    int p = lpSharedMemory->downloadProgress/10;
//    QString progress = QString("#").repeated(p)+QString(" ").repeated(10-p)+" "+QString::number(lpSharedMemory->downloadProgress)+"%";
//    qDebug() << progress;
}
