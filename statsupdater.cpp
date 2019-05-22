#include "statsupdater.h"
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QThread>
#include <QProcess>
#include <QCryptographicHash>
#include <QProcess>
#include "vdf_parser.hpp"
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace tyti;

StatsUpdater::StatsUpdater()
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));

    log.installLog(QCoreApplication::applicationDirPath()+"\\update.log");
}
StatsUpdater::~StatsUpdater()
{
    log.finishLog();
}

void StatsUpdater::start()
{
    int local_version, global_version;
    QSettings settings("stats.ini", QSettings::IniFormat);
    local_version = settings.value("info/version", "0.0.0").toString().remove(".").toInt();
    bool enableUpdates = settings.value("settings/enableUpdates", true).toBool();
    bool downloadMaps = settings.value("settings/downloadMaps", true).toBool();
    server_addr = settings.value("info/serveraddr", server_addr).toString();
    QString url = server_addr+"/update.php?sid="+get_steam_id()+"&key=" + QLatin1String(SERVER_KEY)+"&";
    ss_path = get_soulstorm_installlocation();
    bool enableWXPSP3Compability = settings.value("settings/enableWXPSP3Compability", true).toBool();

    Request request(url);
    RequestSender sender;
    sender.setUserAgent("StatsUpdater");
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("http://www.google.com"));
    QNetworkReply *reply = nam.get(req);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    bool internet = false;
    if(!reply->bytesAvailable())
        qDebug() << "Player are not connected to the internet";
    else{
        qDebug() << "Player are connected to the internet";
        internet = true;
    }

    global_version = QString::fromUtf8(internet?sender.getWhileSuccess(request).data():"").toInt();

    qDebug() << "server address:" << server_addr;
    qDebug() << "local version:" << local_version;
    qDebug() << "global version:" << global_version;

    request.setAddress(url+"name=ssstats.md5&");
    QByteArray btar = internet?sender.getWhileSuccess(request):QByteArray();
    QStringList md5 = QString::fromUtf8(btar.data()).split("\n");
    md5.removeAll("");
    bool success = true, force_update=false;

    if(!enableUpdates)
        qDebug() << "UPDATES DISABLED";

    foreach(QString fileInfo, md5){
        int i = md5.indexOf(fileInfo);
        QString fileName = fileInfo.split("*").at(1);
        fileName.remove("\n");
        fileName.remove("\r");
        if(!QFile::exists(fileName)){
            qDebug() << fileName << "does not exists!";
            force_update = true;
        }
        else{
            if(fileName!="stats.ini"){
                if(md5.at(i).contains(calcMD5(fileName))){
                    qDebug() << "md5 of" << fileName << "is ok!";
                    md5.removeAt(i);
                }
                else {
                    qDebug() << "md5 of" << fileName << "md5 is not ok!";
                    force_update = true;
                }
            }else{
                if(!settings.contains("info/serverAddr"))
                    continue;
                if(!settings.contains("info/version"))
                    continue;
                if(!settings.contains("settings/enableDXHook"))
                    settings.setValue("settings/enableDXHook", 0);
                if(!settings.contains("settings/enableStats"))
                    settings.setValue("settings/enableStats", 1);
                if(!settings.contains("settings/sendReplays"))
                    settings.setValue("settings/sendReplays", 1);
                if(!settings.contains("settings/disableFog"))
                    settings.setValue("settings/disableFog", 0);
                if(!settings.contains("settings/showHP"))
                    settings.setValue("settings/showHP", 0);
                if(!settings.contains("settings/enableUpdates"))
                    settings.setValue("settings/enableUpdates", 1);
                if(!settings.contains("settings/downloadMaps"))
                    settings.setValue("settings/downloadMaps", 1);
                if(!settings.contains("settings/showMenu"))
                    settings.setValue("settings/showMenu", 1);
                if(!settings.contains("settings/showRaces"))
                    settings.setValue("settings/showRaces", 1);
                if(!settings.contains("settings/showAPM"))
                    settings.setValue("settings/showAPM", 1);
                if(!settings.contains("hotkeys/showMenuHK"))
                    settings.setValue("hotkeys/showMenuHK", "Ctrl+M");
                if(!settings.contains("hotkeys/showRaceHK"))
                    settings.setValue("hotkeys/showRaceHK", "Ctrl+R");
                if(!settings.contains("hotkeys/showAPMHK"))
                    settings.setValue("hotkeys/showAPMHK", "Ctrl+A");
                qDebug() << "stats.ini is ok!";
                md5.removeAt(i);
            }
        }
    }
    sender.setMaxWaitTime(60000);
    if((global_version>local_version||force_update)&&enableUpdates&&internet){
        foreach(QString fileInfo, md5){
            QString fileName = fileInfo.split("*").last();
            fileName.remove("\n");
            fileName.remove("\r");
            request.setAddress(url+"name="+fileName+"&");
            int error_code = 0;
            qDebug() << "Downloading" << fileName << "...";
            QByteArray btar = sender.getWhileSuccess(request);
            if(!btar.isEmpty()&&fileInfo.contains(calcMD5(btar))){
                QFile::remove("t_"+fileName);
                QFile cur_file("t_"+fileName);
                if(cur_file.open(QIODevice::WriteOnly))
                {
                    cur_file.write(btar);
                    cur_file.close();
                    if(QFile::exists(fileName)){
                        if(QFile::remove(fileName)){
                            if(QFile::rename("t_"+fileName, fileName))
                                qDebug() << fileName << "successfully updated 4";
                            else
                                error_code = 4;
                        }else if(fileName=="SSStats.exe"){
                            systemWin32 processes;
                            int result = 0;
                            if(processes.findProcess(fileName)){
                                processes.closeProcessByName(fileName);
                                Sleep(5000);
                                processes.updateProcessList();
                            }
                            if(processes.findProcess(fileName)){
                                result = QProcess::execute("taskkill", {"/F", "/IM", fileName});
                                Sleep(5000);
                            }
                            if(!result&&QFile::remove(fileName)){
                                if(QFile::rename("t_"+fileName, fileName))
                                    qDebug() << fileName << "successfully updated 9";
                                else
                                    error_code = 9;
                            }else{
                                updateExecutable(fileName, true);
                                error_code = 8;
                            }
                        }else if(fileName == "SSStatsUpdater.exe"){
                            if(updateExecutable(fileName, true)){
                                qDebug() << fileName << "successfully updated 6";
                                return;
                            }
                            else
                                error_code = 6;
                        }else if(updateExecutable(fileName))
                            qDebug() << fileName << "successfully updated 5";
                        else
                            error_code = 5;
                    }else
                        if(QFile::rename("t_"+fileName, fileName))
                            qDebug() << fileName << "successfully updated 3";
                        else
                            error_code = 3;
                }else
                    error_code = 2;
            }else
                error_code = 1;

            if(error_code!=0)
            {
                qDebug() << "Failed to update" << fileName << "code" << error_code;
                success = false;
            }
        }
        if(success&&global_version){
            QString version = QString::number(global_version);
            version = version.insert(1, '.').insert(3, '.');
            settings.setValue("info/version", version);
        }
    }

    QProcess schtasks;
    QTextCodec *codec = QTextCodec::codecForName("CP866");
    // проверим наличие старой задачи в планировщике и удалим ее если она есть
    if(QProcess::execute("schtasks", {"/tn", "Soulstorm Ladder"})==0)
        QProcess::execute("schtasks", {"/delete", "/tn", "Soulstorm Ladder", "/F"});
    // проверим наличие задачи в планировщике
    schtasks.start("schtasks", {"/tn", "Soulstorm Ladder v2.0"});
    schtasks.waitForFinished();
    // если результат не 0, то задачи нет, создадим ее
    if(schtasks.exitCode()){
        qDebug() << "Adding a launch task to the Task Scheduler";
        QFile file(":/SSStatsSchTask.xml");
        if(file.open(QIODevice::ReadOnly))
        {
            QString UserId = QString(QString(getenv("USERDOMAIN"))+"\\"+QString(getenv("USERNAME")));
            QString Command = "\""+QCoreApplication::applicationDirPath()+"/SSStatsUpdater.exe\"";
            QString WDir = QCoreApplication::applicationDirPath();
            QDomDocument domDoc;
            QString errorMsg;
            int errLine, errCol;
            if(domDoc.setContent(&file, &errorMsg, &errLine, &errCol))
            {
                file.close();
                QDomText UserIdDomText = domDoc.createTextNode(UserId);
                QDomText CommandDomText = domDoc.createTextNode(Command);
                QDomText WDirDomText = domDoc.createTextNode(WDir);
                QDomElement domElement = domDoc.documentElement();
                traverseNode(domElement, "UserId", UserIdDomText);
                traverseNode(domElement, "Command", CommandDomText);
                traverseNode(domElement, "WorkingDirectory", WDirDomText);

                QFile new_xml("temp.xml");
                if(new_xml.open(QIODevice::WriteOnly))
                {
                    QTextStream in(&new_xml);
                    in.setGenerateByteOrderMark(true);
                    in.setCodec("UTF-16LE");
                    in << domDoc.toString().replace("\n","\r\n");
                    new_xml.close();
                }
            }
            else
                qDebug() << errorMsg << errLine << errCol;

            if(QSysInfo::windowsVersion()<0x00a0)
                schtasks.start("schtasks", {"/create", "/tn", "Soulstorm Ladder v2.0", "/xml", "temp.xml"});
            else
                schtasks.start("schtasks", {"/create", "/tn", "Soulstorm Ladder v2.0", "/xml", "temp.xml", "/HRESULT"});
            schtasks.waitForFinished();
            qDebug() << codec->toUnicode(schtasks.readAllStandardOutput()).remove("\r\n")
                     << QString("%1").arg((ulong)schtasks.exitCode(),8,16,QLatin1Char('0')).toUpper();
            QFile::remove("temp.xml");
        }
    }else
        qDebug() << "Shctasks return code for query Soulstorm Ladder:"
                 << QString("%1").arg((ulong)schtasks.exitCode(),8,16,QLatin1Char('0')).toUpper();
    if(enableWXPSP3Compability){
    // установим режим совместимости c Windows XP SP3
    // так как только в этом режиме оверлей работает как положенно
    QProcess regedit;
    regedit.start("reg", {"add", "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers",
                                 "/v", ss_path.replace("/","\\") +"\\Soulstorm.exe", "/t", "REG_SZ", "/d", "~ WINXPSP3 DWM8And16BitMitigation", "/f" });
    regedit.waitForFinished();
    qDebug() << codec->toUnicode(regedit.readAllStandardOutput()).remove("\r\n")
             << QString("%1").arg((ulong)regedit.exitCode(),8,16,QLatin1Char('0')).toUpper();
    }

    // добавим эту программу в автозагрузку текущего пользователя
    QSettings settings_Run("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if(!settings_Run.contains("SSStatsUpdater")){
        settings_Run.setValue("SSStatsUpdater", "\""+QDir::toNativeSeparators(QCoreApplication::applicationFilePath())+"\"");
        settings_Run.sync();
    }
    QSettings settings_App_Paths("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\SSStatsUpdater.exe", QSettings::NativeFormat);
    settings_App_Paths.setValue("Default", "\""+QDir::toNativeSeparators(QCoreApplication::applicationFilePath())+"\"");
    settings_App_Paths.setValue("Path", "\""+QDir::toNativeSeparators(QCoreApplication::applicationDirPath())+"\"");
    settings_App_Paths.sync();
    // запустим сборщик статистики
    if(QProcess::startDetached("SSStats.exe", {}, QCoreApplication::applicationDirPath()))
        qDebug() << "Stats started successfully";
    else
        qDebug() << "Stats failed to start";

    // загрузка маппака и отдельных карт из него
    // если маппак не загружен, то загружаем его
    if(downloadMaps&&internet)
    {
        // иначе получаем список карт с их CRC32 и проверяем карты по списку
        // если CRC32 какой-то карты не совпадает, то обновляем эту карту
        // если карты из списка нет в директории, то загружаем эту карту
        request.setAddress(url+"filetype=map&name=SSStatsMapPack.sfv&");
        QByteArray btar = sender.getWhileSuccess(request);
        QStringList SSStatsMapPackCRC32 = QString::fromUtf8(btar.data()).split("\r");
        SSStatsMapPackCRC32.removeAll("");
        foreach(QString line, SSStatsMapPackCRC32){
            if(line.at(0)==';') continue;
            line.remove("\n");
            QString fileName = QFileInfo(line.split(" ").first()).fileName();
            QString path = QFileInfo(line.split(" ").first()).absolutePath();
//            qDebug() << (path+"/"+fileName+".sgb");
            QFile map(path+"/"+fileName);
            fileName = fileName.remove(QRegExp("\\.\\w{3}"));
//            qDebug() << fileName;
            if(map.exists()){
                if(map.open(QIODevice::ReadOnly)){
                    QString temp_crc = CRC32fromIODevice(&map).toUpper();
                    if(temp_crc!=line.split(" ").last()){
                        qDebug() << fileName << temp_crc << line.split(" ").last();
                        map.close();
                        download_map(fileName, path);
                    }
                }
            } else {
                qDebug() << "Map" << fileName << "does not exists" << line.split(" ").last();
                download_map(fileName, path);
            }
        }
    }
    send_logfile(global_version);
}

QString StatsUpdater::get_soulstorm_installlocation()
{
    QString path = QCoreApplication::applicationDirPath();
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

void StatsUpdater::download_map(QString map_name, QString path)
{
    if(map_name.isEmpty())return;
    qDebug() << "Downloading" << map_name;
    QString url_str = server_addr+"/update.php?sid="+get_steam_id()+"&key=" + QLatin1String(SERVER_KEY);
    url_str += "&filetype=map&name="+QUrl::toPercentEncoding(map_name,""," ")+".zip";

    Request request(url_str);
    RequestSender sender;
    QByteArray btar = sender.getWhileSuccess(request);
    QString fileName = map_name+".zip";
    if(!btar.isEmpty()){
        QFile cur_file(fileName);
        if(cur_file.open(QIODevice::WriteOnly)){
            cur_file.write(btar);
            cur_file.close();
            qDebug() << fileName << "Downloaded successfully";
            if(RequestSender::decompress(cur_file.fileName(),path,"")){
                qDebug() << fileName << "Unpacked successfully";
                QFile::remove(fileName);
            }else qDebug() << "Failed to unpack" << fileName;
        }else qDebug() << "Could not open" << fileName;
    }else qDebug() << "Reply is empty";
}

void StatsUpdater::traverseNode(const QDomNode& node, QString tagName, QDomText domText)
{
    QDomNode domNode = node.firstChild();
    while(!domNode.isNull()) {
        if(domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            if(!domElement.isNull())
                if(domElement.tagName()==tagName)
                {
//                    qDebug() << "TagName: " << domElement.tagName()
//                    << "\tText: " << domElement.text();
                    domElement.replaceChild(domText, domElement.firstChild());
//                    qDebug() << "TagName: " << domElement.tagName()
//                    << "\tText: " << domElement.text();
                }
        }
        traverseNode(domNode, tagName, domText);
        domNode = domNode.nextSibling();
    }
}


void StatsUpdater::send_logfile(int version)
{
    RequestSender sender;
    QString filename = get_steam_id();
    QFile log("update.log");
    if(log.open(QIODevice::ReadOnly))
    {
        QString url = server_addr+
                "/logger.php?key="+QLatin1String(SERVER_KEY)+
                "&steamid="+filename+
                "&version="+QString::number(version)+
                "&type=2";
        Request request;
        request.setAddress(url);
        request.setFile(log.readAll(),filename+".log","text/txt");
        qDebug() << "Reply from server:" << sender.post(request);
//        sender->post(request);url, filename+".log", "text/txt", log.readAll());
        log.close();
    }
}

QString StatsUpdater::get_steam_id()
{
    if(!sender_steamID.isEmpty())
        return sender_steamID;
    QString sid;
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Valve\\Steam\\", QSettings::NativeFormat);
    QString steam_path =  settings.value("InstallPath").toString();
    if(steam_path.isEmpty())
    {
        QSettings settings_second("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
        steam_path = settings_second.value("SteamPath").toString();
    }
    qDebug() << steam_path;
    int Timestamp = 0;
    std::ifstream file(QString(steam_path+"/config/loginusers.vdf").toStdString());
    vdf::object root = vdf::read(file);
    for (auto it : root.childs)
    {
        int temp = QString::fromStdString(it.second->attribs["Timestamp"]).toInt();
        if(temp>Timestamp)
        {
            sid = QString::fromStdString(it.first);
            Timestamp = temp;
        }
    }
    sender_steamID = sid;
    qDebug() << "Player's Steam ID associated with Soulstorm:" << sender_steamID;
    return sender_steamID;
}

QString StatsUpdater::calcMD5(QString fileName)
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

QString StatsUpdater::CRC32fromIODevice( QIODevice * device )
{
    quint32 crc32 = 0xffffffff;
    QByteArray buf(256, 0);
    uint n;
    while( ( n = device->read( buf.data(), 256) ) > 0 )
        for ( uint i = 0; i < n; i++ )
            crc32 = ( crc32 >> 8 ) ^ CRC32Table[(crc32 ^ buf.at(i)) & 0xff ];

    crc32 ^= 0xffffffff;
    return QString("%1").arg(crc32, 8, 16, QChar('0'));
}

QString StatsUpdater::CRC32fromByteArray( const QByteArray & array )
{
    quint32 crc32 = 0xffffffff;
    QByteArray buf(256, 0);
    qint64 n;
    QBuffer buffer;
    buffer.setData( array );
    if ( !buffer.open( QIODevice::ReadOnly ) )
        return 0;
    while( ( n = buffer.read( buf.data(), 256) ) > 0 )
        for ( qint64 i = 0; i < n; i++ )
            crc32 = ( crc32 >> 8 ) ^ CRC32Table[(crc32 ^ buf.at(i)) & 0xff ];
    buffer.close();
    crc32 ^= 0xffffffff;
    return QString("%1").arg(crc32, 8, 16, QChar('0'));
}


QString StatsUpdater::calcMD5(QByteArray data)
{
    QString result;
    QCryptographicHash cryp(QCryptographicHash::Md5);
    cryp.addData(data);
    result = cryp.result().toHex().data();
    return result;
}

bool StatsUpdater::updateExecutable(QString name, bool start_after)
{
    qDebug() << "updateExecutable" << name;
    std::string batfile = name.left(name.size()-4).toStdString()+"updater.bat";
    std::string filename = name.toStdString();
    std::ofstream out(batfile);
    out << "@echo off\n";
    out << ":try\n";
    out << "timeout 1\n";
    out << "del " << filename << "\n";
    out << "if exist " << filename << " goto try\n";
    out << "ren t_" << filename << " " << filename << "\n";
    if(start_after)
        out << filename << "\n";
    out << "del " << batfile;
    out.close();

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;

    ZeroMemory( &si, sizeof(si) );
    ZeroMemory( &pi, sizeof(pi) );
    si.cb = sizeof(si);
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW;

    QString command = "cmd.exe /c "+QString::fromUtf8(batfile.data(), batfile.size());
    qDebug() << command;
    DWORD iReturnVal = 0;

    if(CreateProcessA(NULL, (LPSTR)command.toStdString().c_str(), NULL, NULL, false,
    /*IDLE_PRIORITY_CLASS|*/CREATE_NO_WINDOW|CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_CONSOLE/*|DETACHED_PROCESS*/, NULL, NULL, &si, &pi))
        qDebug() << "process created";
    else
    {
        iReturnVal = GetLastError();
        qDebug() << "process was not created" << iReturnVal;
    }
    return iReturnVal==0;
}
