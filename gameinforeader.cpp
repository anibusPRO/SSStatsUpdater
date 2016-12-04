#include "gameinforeader.h"
#include <QFile>
#include <QDir>
#include "requestsender.h"
#include <QVariantList>
#include <QCoreApplication>
#include <QSettings>

using namespace Network;

GameInfoReader::GameInfoReader()
{
    _game_info = 0;
}

GameInfoReader::~GameInfoReader()
{
    if(_game_info) delete _game_info;
    if(rep_reader) delete rep_reader;
}
bool GameInfoReader::timeCompare(QTime t1, QTime t2)
{
    if((t1.hour()>t2.hour()))
        return true;
    if((t1.hour()==t2.hour())&&(t1.minute()>t2.minute()))
        return true;
    return false;
}

bool GameInfoReader::isGameGoing()
{
//    QTime t_stopgame;
//    QString stopgame_time_str = read_warnings_log("Game Stop", -3);
//    if(stopgame_time_str.isNull())
//        return false;
//    if(stopgame_time_str != "")
//        t_stopgame = QTime::fromString(stopgame_time_str, "hh:mm:ss.z");
//    else t_stopgame = QTime::fromString("00:00:00.00", "hh:mm:ss.z");
//    last_stopgame = t_stopgame;
//    qDebug() << read_warnings_log("Game Start", -3);
//    qDebug() << last_startgame.toString() << last_stopgame.toString();
//    qDebug() << (last_startgame>last_stopgame);
    if(!last_startgame.isNull())
        return last_startgame>last_stopgame;
    return false;
//    QString startgame_time_str = read_warnings_log("Game Start", -3);
//    if(startgame_time_str.isNull()||startgame_time_str== "")
//        return false;
//    QTime t_startgame = QTime::fromString(startgame_time_str, "hh:mm:ss.z");

//    return (t_startgame>t_stopgame);
}


bool GameInfoReader::isPlayback()
{
    read_warnings_log("Game Start", -3);
    qDebug() << last_startgame.toString() << last_playback.toString();
    return last_startgame<last_playback;
//    QString playback_time_str = read_warnings_log("Game Playback", -3);
//    if(playback_time_str.isNull())
//        return false;
//    QTime t_playback = QTime::fromString(playback_time_str, "hh:mm:ss.z");
//    last_playback = t_playback;
//    QString gamestart_time_str = read_warnings_log("Game Start", -3);
//    if(gamestart_time_str.isNull())
//        return false;
//    QTime t_startgame = QTime::fromString(gamestart_time_str, "hh:mm:ss.z");
//    last_startgame = t_startgame;

//    return (t_startgame>t_playback);

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

QByteArray GameInfoReader::get_playback_file()
{
    return _playback;
}

void GameInfoReader::setAverageAPM(int apm)
{
    average_apm = apm;
}

GameInfo *GameInfoReader::get_game_info(QString profile, QString path_to_playback)
{
    QByteArray data;
    qDebug() << profile;
    qDebug() << path_to_playback;
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
    file.close();
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
            if((!winby.isNull())&&winby!="")
            {
                // добавим условие победы
                _game_info->set_winby(winby);
                qDebug() << "WinBy:" << winby.toUpper();
            }
            else
                if(duration<MINIMUM_DURATION)
                {
                    qDebug() << "The current game is still in progress";
                    return 0;
                }
                else
                {
                    // запишим условие победы как дисконнект
                    _game_info->set_winby("disconnect");
                    qDebug() << "WinBy:" << "Disconnect";
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
//                if(team_num!=2)
//                {
//                    qDebug() << "Unsupported type of game."<<"Count of teams:"<<team_num;
//                    return 0;
//                }
            }
            else
            {
                qDebug() << "GSGameStats does not contain Teams";
                return 0;
            }

            QString sender_name = get_sender_name();

            if(!sender_name.isNull())
                _game_info->set_sender_name(sender_name);
            else
            {
                qDebug() << "sender name is null";
                return 0;
            }
            QString mod_name = read_warnings_log("MOD -- Initializing Mod", 4);
            qDebug() << "Mod name is" << mod_name;
            // добавим название последнего запущенного мода к информации об игре
            _game_info->set_mod_name(mod_name);

            qDebug() << "playback reading";
//            // откроем реплей файл последней игры для чтения
            QFile pfile(path_to_playback+"temp.rec");
            if(!pfile.open(QIODevice::ReadWrite))
            {
                qDebug() << "Could not open temp.rec";
                return 0;
            }
            rep_reader = new RepReader;
            // создадим поток данны из файла
            QDataStream in(&pfile);
            // прочитаем реплей
//            try{
            rep_reader->ReadReplayFully(&in, pfile.fileName());
            // переименовываем название реплея в игре по стандарту
            rep_reader->RenameReplay();
            pfile.seek(0);
            _playback = pfile.readAll();
//            }
//            catch(...)
//            {
//                playback.close();
//            }
            pfile.close();

            // если настройки игры стандартные, то отправим статистику
            int team_1_p_count=0, team_2_p_count=0, first_team_id=0;
            if(rep_reader->isStandart())
            {
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
                                            ++team_2_p_count;
                                    int apm = 0;
                                    if(player.value("PName")==sender_name)
                                        apm = average_apm;
                                    else apm = 0;
                                    _game_info->add_player(player.value("PName").toString(), player.value("PRace").toString(),
                                                           player.value("PTeam").toInt(), player.value("PFnlState").toInt(), apm/*rep_reader->GetAverageAPM(i)*/);


                                }
                                else
                                {

                                    qDebug() << "Player does not contain players info";
                                    return 0;
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
            }
            else
            {
                qDebug() << "Game options is not standart";
                return 0;
            }
//            QString str = rep_reader->RenameReplay();
//            qDebug() << str;
//            playback.close();

//            if(playback.rename(str+".rec"))
//                qDebug() << "Rep file renamed successfully";
//            else
//                qDebug() << "Error while rename rep file";


            if(team_1_p_count==team_2_p_count)
                _game_info->set_type(team_1_p_count);
            else
//                _game_info->set_type(4);
            {
                qDebug() << "invalid game type";
            }
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

    if(userdata_dir.cd(steam_path + "/userdata"))
    {
        if(userdata_dir.entryInfoList().size()>2)
        {
            qDebug() << userdata_dir.path();
            foreach (QString name, userdata_dir.entryList())
            {
                if(name!="."&&name!="..")
                {
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
            if(!account_id32_str.isNull())
            {
                account_id32 = account_id32_str.toInt();
                quint64 account_id64 = 76561197960265728 + account_id32;

                QString steam_id64 = QString::number(account_id64);
                if(_game_info!=0) _game_info->set_steam_id(steam_id64);
                Request request("http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=" + QLatin1String(STEAM_API_KEY) + "&steamids="+steam_id64+"&format=json");
                QVariantMap player_info = QtJson::json_to_map(sender.get(request));
                if(player_info.contains("response"))
                {
                    qDebug() << "response";
                    QVariantMap response = player_info.value("response").toMap();
                    if(response.contains("players"))
                    {
                        qDebug() << "players";
                        QVariantList players = response.value("players").toList();
                        if(!players.isEmpty())
                            if(players.at(0).toMap().contains("personaname"))
                            {
                                qDebug() << "personaname";
                                player_name = players.at(0).toMap().value("personaname").toString();
                                if(init)
                                {
                                    qDebug() << "init";
                                    qDebug() << player_name;
                                    QByteArray btr = player_name.toUtf8();
                                    QString hex_name(btr.toHex());

                                    Request request_to_server("http://tpmodstat.16mb.com/regplayer.php?name="+hex_name+"&sid="+steam_id64+"&key="+QLatin1String(SERVER_KEY));
                                    QByteArray response_fromstat = sender.get(request_to_server);

                                    QString r_str = QString::fromUtf8(response_fromstat.data());
                                    qDebug() << r_str;
                                    if(response_fromstat.isNull())
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
    QString out="";

    QFile file(ss_path+"/warnings.log");

    // запишем время первой попытки открытия warnings.log
    QTime time = QTime::currentTime();
//    qDebug() << "open warning log";
    while(!file.open(QIODevice::ReadOnly))
    {
        // если с первого раза лог не котрылася попробуем через 1 секунду пока не откроется
        // если через 10 секунд лог открыть не удалось, то вернем пустую строку
        if(QTime::currentTime().toString("s").toInt() - time.toString("s").toInt()>10)
        {
            qDebug() << "Could not open warnings.log";
            return QString();
        }
    }

    QTextStream textStream(&file);

    QString str_stop = "Game Stop";
    QString str_start = "Game Start";
    QString str_playback = "Game Playback";
    bool stop = false;
    int state_offset = -3;
    qDebug() << "find string";
    QStringList fileLines = textStream.readAll().split("\n");
    int counter = fileLines.size();
    while (counter!=0)
    {
//        qDebug() << "read next line" << counter-1;
        // читаем следующую строку из файла
        QString line = fileLines.at(counter-1);
        // если текущаю строка содержит переданную в параметрах строку
        if(line.contains(str, Qt::CaseInsensitive))
        {
            // результат поиска
            out="";
            // удаляем запятые из прочитанной строки
            line = line.remove(',');
            // получаем список строк разделив текущую строку по пробелам
            QStringList list = line.split(" ");
            list.removeAll(""); // удаляем пустые строки из списка
            list.removeAll("\n"); // удаляем переводы строк
            list.removeAll("\r");
            // если строка сложная, то есть содержит более одного слова через пробел
            // то получим первое слово в строке и прорим по нему
            // если переданная строка содержит пробелы,
            QString first_word;
            if(str.contains(" ", Qt::CaseInsensitive))
                // делим переданную строку через пробел, получаем первый элемент списка
                // к примеру если передали Game Start, то получим Game
                first_word = str.split(" ")[0];

            // если в списке полученном из текущей строки есть искомое слово
            if(list.contains(first_word, Qt::CaseInsensitive))
            {
                // пишем в вывод строку из списка с отступом, переданном в параметре
                if(list.indexOf(first_word, Qt::CaseInsensitive)!=-1)
                    out = list[list.indexOf(first_word, Qt::CaseInsensitive)+offset];
                else
                    qDebug() << "out of range" << first_word << counter-1;
            }
        }
        if(!stop&&(line.contains(str_stop , Qt::CaseInsensitive)||
           line.contains(str_start, Qt::CaseInsensitive)||
           line.contains(str_playback, Qt::CaseInsensitive)))
        {
            // удаляем запятые из прочитанной строки
            line = line.remove(',');
            // получаем список строк разделив текущую строку по пробелам
            QStringList list = line.split(" ");
            list.removeAll(""); // удаляем пустые строки из списка
            list.removeAll("\n"); // удаляем переводы строк
            list.removeAll("\r");

            if(list.contains("Game", Qt::CaseInsensitive))
            {
                if(list.indexOf("Game", Qt::CaseInsensitive)!=-1)
                {
                    if(list.at(4).contains("Stop", Qt::CaseInsensitive))
                        last_stopgame = QTime::fromString(list[list.indexOf("Game", Qt::CaseInsensitive)+state_offset], "hh:mm:ss.z");
                    if(list.at(4).contains("Start", Qt::CaseInsensitive))
                    {
                        last_startgame = QTime::fromString(list[list.indexOf("Game", Qt::CaseInsensitive)+state_offset], "hh:mm:ss.z");
                        stop = true;
                    }
                    if(list.at(4).contains("Playback", Qt::CaseInsensitive))
                        last_playback = QTime::fromString(list[list.indexOf("Game", Qt::CaseInsensitive)+state_offset], "hh:mm:ss.z");
                }
                else
                    qDebug() << "out of range" << "Game" << counter-1;;
            }
        }
        --counter;
    }
    file.close();
    return out;
}
