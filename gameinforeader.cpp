#include "gameinforeader.h"
#include <QFile>
#include <QDir>
#include "requestsender.h"
#include <QVariantList>
#include <QCoreApplication>
#include <QTime>
#include <QSettings>
using namespace Network;

GameInfoReader::GameInfoReader()
{
    _game_info = 0;
}

GameInfoReader::~GameInfoReader()
{
    if(_game_info!=0)
        delete _game_info;
}

bool GameInfoReader::isPlayback()
{
    QTime t_playback = QTime::fromString(read_warnings_log("Game Playback", -3), "hh:mm:ss.z");
    QTime t_gamestart = QTime::fromString(read_warnings_log("Game Start", -3), "hh:mm:ss.z");
    if((t_gamestart.minute()-t_playback.minute())<2)
        return true;
    else
        return false;
}

QString GameInfoReader::get_cur_profile_dir(QString path)
{
    QDir temp_dir(path);
//    temp_dir.cdUp();

    QString name = read_warnings_log("profile", 1);
    temp_dir.cd("Profiles");

    QStringList profiles = temp_dir.entryList();
    QString profile;
    if(!profiles.isEmpty())
        foreach (QString file, profiles)
        {
//            qDebug() <<  file;
            if(file!="."&&file!="..")
            {
                temp_dir.cd(file);
//                qDebug() << temp_dir.path()+"/name.dat";
                QFile temp(temp_dir.path()+"/name.dat");

                if (!temp.open(QFile::ReadOnly | QFile::Text))
                {
                    qDebug() << "Could not open name.dat";
    //                return 0;
                }
                QTextStream in(&temp);
                QString str = in.readAll();
//                qDebug() << str;
                if(name == str)
                    profile = file;
                temp_dir.cdUp();
            }
        }
    return profile;
}

GameInfo *GameInfoReader::get_game_info(QString profile)
{
    QByteArray data;

    QFile file(profile + "/testStats.lua");

    qDebug() << "Getting the game data";
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open testStats.lua";
        return 0;
    }
    if(!file.isReadable())
    {
        qDebug() << "testStats.lua is not readable";
        return 0;
    }
    // в начале файла лежит 3 байта какой то шняги из за этого корневой ключ не читается
    // пропустим эту шнягу
    int i=0;
    while(file.read(1)!="G")
        i++;
    file.seek(i);

    data = file.readAll();

    QVariantMap stats = QtJson::to_map(data);
    // но у меня все было ок, поэтому я на всякий случай сделаю проверку без 3 байт полезных данных
    if(stats.contains("GSGameStats"))
    {
        QVariantMap GSGameStats = stats.value("GSGameStats").toMap();
        if(GSGameStats.contains("WinBy"))
        {
            int players_count = 0;
            if(GSGameStats.contains("Players"))
                players_count = GSGameStats.value("Players").toInt();
            else
                qDebug() << "GSGameStats does not contain Players";

            _game_info = new GameInfo(players_count);
            QString winby = GSGameStats.value("WinBy").toString();

            int duration=0;
            if(GSGameStats.contains("Duration"))
            {
                duration = GSGameStats.value("Duration").toInt();
                _game_info->set_duration(duration);
            }
            else
                qDebug() << "GSGameStats does not contain Duration";

            // если условие победы не пустое
            if(!winby.isNull()&&winby!="")
            {
                // добавим условие победы
                _game_info->set_winby(winby);
                qDebug() << "WinBy:" << winby.toUpper();
            }
            else
                if(duration<30)
                {
                    qDebug() << "The current game is still in progress";
                    return 0;
                }
                else
                {
                    // запишим условие победы как дисконнект
                    _game_info->set_winby("disconnect");
                    qDebug() << "WinBy:" << winby.toUpper();
                }

            if(GSGameStats.contains("Scenario"))
                _game_info->set_map_name(GSGameStats.value("Scenario").toString());
            else
                qDebug() << "GSGameStats does not contain Scenario";

            int team_num = 0;
            if(GSGameStats.contains("Teams"))
            {
                team_num = GSGameStats.value("Teams").toInt();
                _game_info->set_team_number(team_num);
                if(team_num!=2)
                {
                    qDebug() << "Unsupported type of game."<<"Count of teams:"<<team_num;
                    return 0;
                }
            }
            else
            {
                qDebug() << "GSGameStats does not contain Teams";
                return 0;
            }

            int team_1_p_count=0, team_2_p_count=0, first_team_id=0;
            for(int i=0; i<players_count;++i)
            {
                QString player_id = "player_"+QString::number(i);
                if(GSGameStats.contains(player_id))
                {
                    QVariantMap player = GSGameStats.value(player_id).toMap();
                    if(player.contains("PHuman"))
                    {
                        if(player.value("PHuman").toInt()==1)
                        {
                            if(player.contains("PName")&&player.contains("PRace")
                                    &&player.contains("PFnlState")&&player.contains("PTeam"))
                            {
                                if(i==0)
                                {
                                    first_team_id = player.value("PTeam").toInt();
                                    ++team_1_p_count;
                                }
                                else
                                    if(first_team_id==player.value("PTeam").toInt())
                                        ++team_1_p_count;
                                    else
                                        team_2_p_count = player.value("PTeam").toInt();
                                _game_info->add_player(player.value("PName").toString(), player.value("PRace").toString(),
                                                       player.value("PTeam").toInt(), player.value("PFnlState").toInt());
                            }
                            else
                            {
                                return 0;
                                qDebug() << "Player does not contain players info";
                            }
                        }
                        else
                        {
                            qDebug() << "Player is not Human";
                            return 0;
                        }
                    }
                    else
                    {
                        qDebug() << "Player does not contain PHuman";
                        return 0;
                    }
                }
                else
                {
                    qDebug() << "GSGameStats does not contain" << player_id;
                    return 0;
                }
            }
            if(team_1_p_count==team_2_p_count)
                _game_info->set_type(team_1_p_count);
            else
                qDebug() << "invalid game type";

            QString name = get_sender_name();

//            if(!abort.isNull()&& name)


            if(!name.isNull())
                _game_info->set_sender_name(name);
            else
            {
                qDebug() << "sender name is null";
                return 0;
            }
            QString mod_name = read_warnings_log("Initializing", 2);
            qDebug() << "Mod name is" << mod_name;
            // добавим название последнего запущенного мода к информации об игре
            _game_info->set_mod_name(mod_name);
        }
        else
        {
            qDebug() << "GSGameStats does not contain WinBy";
            return 0;
        }
    }
    else
    {
        qDebug() << "testStats.lua does not contain GSGameStats";
        return 0;
    }
    file.close();

    if(_game_info!=0)
        return _game_info;
    return 0;
}

QString GameInfoReader::get_sender_name(bool init/*=false*/)
{
    RequestSender sender;
    sender.setMaxWaitTime(10000);
    QString player_name;

    uint account_id32;
    QString account_id32_str;
    QDir userdata_dir(QCoreApplication::applicationDirPath());

    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Valve\\Steam\\", QSettings::NativeFormat);

    QString steam_path =  settings.value("InstallPath").toString();
    if(steam_path.isNull()||steam_path=="")
    {
        QSettings settings_second("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
        steam_path = settings_second.value("SteamPath").toString();
    }

//    char *s = getenv("ProgramFiles(x86)");
//    QString str(s);
//    qDebug() << str;
    if(userdata_dir.cd(steam_path + "/userdata"))
    {
        if(userdata_dir.entryInfoList().size()>2)
        {
            qDebug() << userdata_dir.path();
            foreach (QString name, userdata_dir.entryList())
            {
                if(name!="."&&name!="..")
                {
//                    qDebug() << name;
                    userdata_dir.cd(name);
                    if(userdata_dir.entryList().contains("9450"))
                    {
                        account_id32_str = name;
                        break;
                    }
                    userdata_dir.cdUp();
                }
            }
            qDebug() << "Steam ID associated with Soulstorm:" << account_id32_str;
//            return QString(account_id32_str);
            if(!account_id32_str.isNull())
            {
                account_id32 = account_id32_str.toInt();
    //            account_id32 = userdata_dir.entryInfoList().at(2).fileName().toInt();
                quint64 account_id64 = 76561197960265728 + account_id32;

                QString steam_id64 = QString::number(account_id64);
                if(_game_info!=0) _game_info->set_steam_id(steam_id64);
                Request request("http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=" + QLatin1String(STEAM_API_KEY) + "&steamids="+steam_id64+"&format=json");
//                qDebug() << "0";
//                qDebug() << sender.get(request);
                QVariantMap player_info = QtJson::json_to_map(sender.get(request));
//                qDebug() << "0.5";
                if(player_info.contains("response"))
                {
//                    qDebug() << "2";
                    QVariantMap response = player_info.value("response").toMap();
                    if(response.contains("players"))
                    {
//                        qDebug() << "3";
                        QVariantList players = response.value("players").toList();
                        if(!players.isEmpty())
                            if(players.at(0).toMap().contains("personaname"))
                            {
//                                qDebug() << "4";
                                player_name = players.at(0).toMap().value("personaname").toString();
                                if(init)
                                {
//                                    QByteArray btr = player_name.toAscii();
//                                    QString hex_name(btr.toHex());
                                    QString hex_name = player_name;
                                    Request request_to_server("http://tpmodstat.16mb.com/regplayer.php?name="+hex_name+"&sid="+steam_id64+"&key="+QLatin1String(SERVER_KEY));
//                                    qDebug() << "request_to_server";
                                    if(sender.get(request_to_server).isNull())
                                        qDebug() << "Server returned empty data";
                                    return QString("initialization");
                                }
                                else
                                    return player_name;
                            }
                            else
                                qDebug() << "Steam response does not contain personaname";
                        else
                            qDebug() << "players list is empty";
                    }
                    else
                        qDebug() << "Steam response does not contain players";
                }
                else
                    qDebug() << "Steam response does not contain response";
            }
            else
            {
                qDebug() << "account_id32_str is Null";
                return QString();
            }
        }
        else
        {
            qDebug() << "userdata folder is empty";
            return QString();
        }
    }
    else
    {
         qDebug() << "userdata is not exists or is not readable";
         return QString();
    }
    qDebug() << "could not change work dir";
    return QString();
}

QString GameInfoReader::read_warnings_log(QString str, int offset/*=0*/)
{
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\thq\\dawn of war - soulstorm\\", QSettings::NativeFormat);

    QString ss_path =  settings.value("installlocation").toString();
    QString out;

    QFile file(ss_path+"/warnings.log");

    // запишем время первой попытки открытия warnings.log
    QTime time = QTime::currentTime();
    while(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open warnings.log";
        // если с первого раза лог не котрылася попробуем через 1 секунду пока не откроется
        // если через 10 секунд лог открыть не удалось, то вернем пустую строку
        if(QTime::currentTime().toString("s").toInt() - time.toString("s").toInt()>10)
            return QString();
    }

    QTextStream textStream(&file);
//    qDebug() << str;
    while (!textStream.atEnd())
    {
        QString line = textStream.readLine();

//        int index;

//        qDebug() << line;
        if(line.contains(str, Qt::CaseInsensitive))
        {
            out="";
            line = line.remove(',');
            QStringList list = line.split(" ");
            list.removeAll("");
            // если строка сложная, то есть содержит более одного слова через пробел
            // то получим первое слово в строке и прорим по нему
            if(str.contains(" ", Qt::CaseInsensitive))
                str = str.split(" ")[0];
            if(list.contains(str, Qt::CaseInsensitive))
            {
                out = list[list.indexOf(str)+offset];
//                qDebug() << out;
            }
//            index = line.indexOf(str);
//            qDebug() << line;
//            qDebug() << 4 << index;
//    //        if(file.right(12).left(2).toInt()>temp_filename.right(12).left(2).toInt())
//            for(int i=index; i<line.length(); ++i)
//                if(line.at(i)==' ')
//                {
//                    ++i;
//                    line.split(' ');
//                    while((!line.at(i).isNull()) ))
//                    {
//                        qDebug() << line.at(i);
//                        out += line.at(i);
////                        qDebug() << out;
//                        ++i;
//                    }
//                    break;
//                }
            break;
        }
    }
    file.close();
    return out;
}
