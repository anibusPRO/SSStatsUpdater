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
    errors_list <<"Stats processed without errors"<<
                  "could not open 'testStats.lua'"<<
                  "testStats.lua is not readable"<<
                  "stats file does not contain 'GSGameStats'"<<
                  "GSGameStats does not contain 'WinBy'"
                  "GSGameStats does not contain 'Duration'"<<
                  "the current game is still in progress"<<
                  "GSGameStats does not contain 'Teams'"<<
                  "Unsupported type of game. The number of teams is not equals two."<<
                  "userdata is not exists or is not readable"<<
                  "userdata folder is empty"<<
                  "account_id32_str is Null"<<
                  "steam response does not contain response"<<
                  "steam response does not contain players"<<
                  "players list is empty"<<
                  "steam response does not contain personaname"<<
                  "could not open 'temp.rec'"<<
                  "GSGameStats does not contain player id"<<
                  "Player does not contain 'PHuman'"<<
                  "Player is not human"<<
                  "Player does not contain players info"<<
                  "Unsupported type of game. Number of players per team is not equal."<<
                  "game options is not standart"<<
                  "sender is not player";
}

GameInfoReader::~GameInfoReader()
{
    if(_game_info) delete _game_info;
//    if(rep_reader) delete rep_reader;
}
// возвращает true если первое время больше второго иначе false
bool GameInfoReader::timeCompare(QTime t1, QTime t2)
{
    if(t1.hour()==23&&t2.hour()<=3)
        return false;
    return t1>t2;
}
// возвращает разницу в минутах между двумя аргументами
int GameInfoReader::timeDifference(QTime t1, QTime t2)
{

    int h1=t1.hour(), h2=t2.hour();
    if(h1==23&&h2==0)
        h2 = 24;
    if(h2==23&&h1==0)
        h1 = 24;
    int dhm = abs(h1 - h2);
    int m = h1>h2 ? (t1.minute() - t2.minute()):(t2.minute() - t1.minute());
    int minutes = abs(dhm*60 + m);

    return minutes;
}

void GameInfoReader::set_ss_path(const QString &value)
{
    ss_path = value;
}

QString GameInfoReader::get_steam_id()
{
    return steam_id64;
}

void GameInfoReader::set_server_addr(QString addr)
{
    server_addr = addr;
}


//bool GameInfoReader::isGameGoing()
//{

////    if(last_startgame.isNull())
//    read_warnings_log("APP -- Game Start", -1);
//    return last_startgame>last_stopgame;
//}

int GameInfoReader::readySend()
{
    read_warnings_log("APP -- Game Start", -1);
    int diff = timeDifference(last_playback, last_startgame);
    if(diff<1)
        return 2;
    if(timeCompare(last_startgame, last_stopgame))
        return 1;
    return 0;
}


//bool GameInfoReader::isPlayback()
//{
//    read_warnings_log("APP -- Game Start", -1);
//    return last_startgame<last_playback;
//}

bool GameInfoReader::tempRecExist()
{
    QString time = read_warnings_log("REC -- Error opening file playback:temp.rec for write", -1);
    return last_startgame>QTime::fromString(time, "hh:mm:ss.z");
}


QString GameInfoReader::get_cur_profile_dir(bool fstart)
{
    QDir temp_dir(ss_path+"/Profiles");

    QString name;
    name = read_warnings_log("Using player profile", 3);
    if(fstart)
        qDebug() << "cur profile name:" << name << cur_profile_name;

    // если имя профиля не изменилось, то отправим пустую строку, как знак того что профиль не сменился
    if(name==cur_profile_name)
        return QString();

    QStringList profiles = temp_dir.entryList();
    QString profile;
    if(!profiles.isEmpty())
        foreach (QString file, profiles)
        {
            if(file!="."&&file!="..")
            {
                temp_dir.cd(file);
                QFile temp(temp_dir.path()+"/name.dat");
                QString str="";
                if (temp.open(QIODevice::ReadOnly))
                {
                    QDataStream in(&temp);
                    int filesize = temp.size()-3;
                    char *bytes = new char[filesize];
                    in.device()->seek(2);
                    in.readRawData(bytes, filesize);

                    str = QString::fromUtf16((ushort*)bytes).left(filesize/2);


                    temp.close();
                    delete[] bytes;
                }
                else
                {
                    qDebug() << temp_dir.path();
                    qDebug() << "Could not open name.dat";
                    temp_dir.cdUp();
                    continue;
                }
                if(name == str)
                {
                    profile = file;
                    cur_profile_name = str;
                    break;
                }
                temp_dir.cdUp();

            }
        }

    // если не удалось получить имя текущего профиля, то используем по умолчанию первый профиль
    if(profile=="") profile = "Profile1";

    return QString(ss_path +"/Profiles/"+ profile);

}

QByteArray GameInfoReader::get_playback_file()
{
    return _playback;
}

void GameInfoReader::setAverageAPM(int apm)
{
    average_apm = apm;
}

QString GameInfoReader::get_url(QString profile, QString path_to_playback)
{
    int e_code = get_game_info(profile, path_to_playback);
    qDebug() << errors_list.at(e_code);
    if(e_code==0)
    {
        QString url;
        if(_game_info!=0)
        {
            url = _game_info->get_url(server_addr+"/connect.php?");
            delete _game_info;
        }
        return url;
    }
    return QString();
}

int GameInfoReader::get_game_info(QString profile, QString path_to_playback)
{
//    qDebug() << profile;
//    qDebug() << path_to_playback;
    QFile file(profile + "/testStats.lua");

    qDebug() << "Getting the game data";
    if(!file.open(QIODevice::ReadOnly))
        return 1;
    if(!file.isReadable())
        return 2;

    // в начале файла лежат байты из-за этого корневой ключ не читается
    // пропустим мусорыне байты

    int i=0;
    while(file.read(1)!="G")
        i++;

    file.seek(i);

    QVariantMap stats = QtJson::to_map(file.readAll());

    file.close();

    if(stats.contains("GSGameStats"))
    {
        QVariantMap GSGameStats = stats.value("GSGameStats").toMap();
        if(GSGameStats.contains("WinBy"))
        {
            // получим количество игроков
            int players_count = 0;
            if(GSGameStats.contains("Players"))
                players_count = GSGameStats.value("Players").toInt();
            else
                qDebug() << "GSGameStats does not contain Players";

            _game_info = new GameInfo(players_count);
            // получим условие победы
            QString winby = GSGameStats.value("WinBy").toString();

            // получим продолжительность игры
            int duration=0;
            if(GSGameStats.contains("Duration"))
            {
                duration = GSGameStats.value("Duration").toInt();
                _game_info->set_duration(duration);
            }
            else
                return 5;

            // а если пустое, то проверим идет ли игра
            if(duration<MINIMUM_DURATION)
                return 6;

            // если условие победы не пустое
            if((!winby.isNull())&&winby!="")
            {
                // добавим условие победы
                _game_info->set_winby(winby);
                qDebug() << "WinBy:" << winby.toUpper();
            }
            else
            {
                // иначе запишим условие победы как дисконнект
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
                if(team_num!=2)
                    return 8;
            }
            else
                return 7;

            QStringList sender_names = get_sender_name();
            if(sender_names.isNull())
            {
                qDebug() << "sender name is null";
                return error_code;
            }

            // ПРОГРАММА БУДЕТ ПОЛУЧАТЬ ИМЯ МОДА ИЗ РЕПЛЕЯ
            // хотя не факт что стоит надеяться на реплей
            // получим имя запущенного мода
            QString mod_name = read_warnings_log("MOD -- Initializing Mod", 4);

            // добавим название последнего запущенного мода к информации об игре
            _game_info->set_mod_name(mod_name);
//            QStringList players;
            // если настройки игры стандартные, то отправим статистику
            int team_1_p_count=0, team_2_p_count=0, first_team_id=0;
            int fnl_state, sender_id;
            QString p_name;
            bool sender_is_player = false;
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

                                p_name = player.value("PName").toString();
                                if(sender_names.contains(p_name))
                                {
                                    _game_info->set_sender_name(p_name);
                                    _game_info->setAPMR(average_apm);
                                    _game_info->set_steam_id(steam_id64.at(sender_names.indexOf(p_name)));
                                    fnl_state = player.value("PFnlState").toInt();
                                    sender_id = i;
                                    sender_is_player = true;
                                }
                                else
                                    if((_game_info->get_winby()=="disconnect")&&(players_count==2))
                                        fnl_state = 5;
                                    else
                                        fnl_state = player.value("PFnlState").toInt();

                                _game_info->add_player(p_name, player.value("PRace").toString(),
                                                           player.value("PTeam").toInt(), fnl_state/*, rep_reader.GetAverageAPM(i)*/);
                            }
                            else
                                return 20;
                        }
                        else
                            return 19;
                    }
                    else
                        return 18;
                }
                else
                {
                    qDebug() << "GSGameStats does not contain" << player_id;
                    return 17;
                }
            }

            if(!sender_is_player)
                return 23;

//            if(playback.rename(str+".rec"))
//                qDebug() << "Rep file renamed successfully";
//            else
//                qDebug() << "Error while rename rep file";
            if(team_1_p_count==team_2_p_count)
                _game_info->set_type(team_1_p_count);
            else
                return 21;

            if(_game_info->get_winby()=="disconnect")
            {
                if(team_1_p_count==1)
                    _game_info->set_winby("ANNIHILATE");
                else
                    if(_game_info->getPlayer(sender_id).fnl_state==0)
                    {
                        int sender_team = _game_info->getPlayer(sender_id).team;
                        int loseleavers=0;
                        for(int i=0; i<players_count; ++i)
                            if((_game_info->getPlayer(i).team == sender_team)&&
                                    (_game_info->getPlayer(i).fnl_state!=1))
                                        ++loseleavers;
                        if(loseleavers==team_1_p_count)
                        {
                            _game_info->set_winby("ANNIHILATE");
                            for(int i=0; i<players_count; ++i)
                                if(_game_info->getPlayer(i).team != sender_team)
                                        _game_info->update_player(i, 5);
                        }
                    }
            }


            _playback = 0;
            RepReader rep_reader;
            if(!tempRecExist())
                qDebug() << "REC -- Error opening file playback:temp.rec for write";
            else
            {
                qDebug() << "playback reading";
                // откроем реплей файл последней игры для чтения
                QFile pfile(path_to_playback+"temp.rec");
                if(!pfile.open(QIODevice::ReadWrite))
                    return 16;

                // создадим поток данны из файла
                QDataStream in(&pfile);

                // прочитаем реплей
                rep_reader.ReadReplayFully(&in, pfile.fileName());
                qDebug() << rep_reader.replay->MOD << mod_name;

                // переименовываем название реплея в игре по стандарту
                rep_reader.RenameReplay();
                pfile.seek(0);
                _playback = pfile.readAll();

                pfile.close();

                if(!rep_reader.isStandart(team_1_p_count))
                    return 22;
            }
        }
        else
            return 4;
    }
    else
    {
        qDebug() << stats;
        return 3;
    }

    return 0;
}

QStringList GameInfoReader::get_sender_name(bool init/*=false*/)
{
    RequestSender sender;
//    sender.setMaxWaitTime(10000);
    QString player_name;
    QStringList retList;
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
                        account_id32 = account_id32_str.toInt();
                        quint64 account_id64 = 76561197960265728 + account_id32;
                        QString account_id64_str = QString::number(account_id64);
                        qDebug() << "Steam ID associated with Soulstorm:" << account_id64 << account_id32;


                        Request request("http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=" + QLatin1String(STEAM_API_KEY) + "&steamids="+account_id64_str+"&format=json");

                        QVariantMap player_info = QtJson::json_to_map(sender.getWhileSuccess(request));
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

                                        if(init)
                                        {
                                            qDebug() << "Steam nickname:" << player_name;
                                            QByteArray btr = player_name.toUtf8();
                                            QString hex_name(btr.toHex());
                                            qDebug() << server_addr;
                                            Request request_to_server(server_addr+"/regplayer.php?name="+hex_name+"&sid="+account_id64_str+"&key="+QLatin1String(SERVER_KEY));
                                            QByteArray response_fromstat = sender.getWhileSuccess(request_to_server);

                                            QString r_str = QString::fromUtf8(response_fromstat.data());
                                            qDebug() << "registration in dowstats: " << r_str;
                                            if(response_fromstat.isNull())
                                                qDebug() << "Server returned empty data";

                                            retList << "initialization";
                                            return retList;
                                        }
                                        else
                                        {
                                            // если мы получили имя игрока, то запишем steam id этого игрока
                                            retList << player_name;
                                            steam_id64 << account_id64_str;
                                            return retList;
                                        }
                                    }
                                    else
                                        error_code = 15;
                                else
                                    error_code = 14;
                            }
                            else
                                error_code = 13;
                        }
                        else
                            error_code = 12;
                    }
                    userdata_dir.cdUp();
                }
            }
        }
        else
            error_code = 10;
    }
    else
        error_code = 9;

    qDebug() << "could not change work dir";
    return QStringList();
}

QString GameInfoReader::read_warnings_log(QString str, int offset/*=0*/)
{
//    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\thq\\dawn of war - soulstorm\\", QSettings::NativeFormat);

//    QString ss_path =  settings.value("installlocation").toString();

    QString out="";

    QFile file(ss_path+"/warnings.log");

    // запишем время первой попытки открытия warnings.log
//    QTime time = QTime::currentTime();
//    qDebug() << "open warning log";
    if(!file.open(QIODevice::ReadOnly))
    {
        // если с первого раза лог не котрылася попробуем через 1 секунду пока не откроется
        // если через 10 секунд лог открыть не удалось, то вернем пустую строку
//        if(QTime::currentTime().toString("s").toInt() - time.toString("s").toInt()>10)
//        {
            qDebug() << "Could not open warnings.log";
            return QString();
//        }
    }

    QTextStream textStream(&file);

    QString str_stop = "Game Stop";
    QString str_start = "Game Start";
    QString str_playback = "Game Playback";
//    QString str_profile = "Using player profile";
    bool fstate = false;
//    bool fprofile = false;
    int state_offset = -3;
//    qDebug() << "find string";
    QStringList fileLines = textStream.readAll().split("\n");
    int counter, end;

    end = 0;
    counter = fileLines.size()-1;

    while (counter!=end)
    {
//        qDebug() << "read next line" << counter-1;
        // читаем следующую строку из файла
        QString line = fileLines.at(counter);
        // если текущаю строка содержит переданную в параметрах строку
        if(line.contains(str, Qt::CaseInsensitive))
        {
            // результат поиска
            out="";

            // удаляем запятые из прочитанной строки
            line = line.remove(',');
            line = line.remove("\n");
            line = line.remove("\r");
            // получаем список строк разделив текущую строку по пробелам
            QStringList list = line.split(" ");
            list.removeAll(""); // удаляем пустые строки из списка

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
                    qDebug() << "out of range" << first_word << str << counter;
            }
        }
        if(!fstate&&(line.contains(str_stop , Qt::CaseInsensitive)||
           line.contains(str_start, Qt::CaseInsensitive)||
           line.contains(str_playback, Qt::CaseInsensitive)))
        {
            line = line.remove(',');
            line = line.remove("\n");
            line = line.remove("\r");
            qDebug() << line;
            QStringList list = line.split(" ");
            list.removeAll("");

            if(list.contains("Game", Qt::CaseInsensitive))
            {
                int index = list.indexOf("Game", Qt::CaseInsensitive);

                if(index!=-1)
                {
                    if(list.at(4).contains("Stop", Qt::CaseInsensitive))
                        last_stopgame = QTime::fromString(list[index+state_offset], "hh:mm:ss.z");
                    if(list.at(4).contains("Start", Qt::CaseInsensitive))
                    {
                        last_startgame = QTime::fromString(list[index+state_offset], "hh:mm:ss.z");
                        fstate = true;
                    }
                    if(list.at(4).contains("Playback", Qt::CaseInsensitive))
                        last_playback = QTime::fromString(list[index+state_offset], "hh:mm:ss.z");
                }
                else
                    qDebug() << "out of range" << "Game" << counter;
            }
        }
//        if(!fprofile&&line.contains(str_profile, Qt::CaseInsensitive))
//        {
//            line = line.remove(',');
//            line = line.remove("\n");
//            line = line.remove("\r");
//            QStringList list = line.split(" ");
//            list.removeAll("");

//            if(list.contains("profile", Qt::CaseInsensitive))
//            {
//                int index = list.indexOf("profile", Qt::CaseInsensitive);
//                if(index!=-1)
//                {
//                    cur_profile_name = list[index+1];
//                    fprofile = true;
//                }
//                else
//                    qDebug() << "out of range" << "profile" << str << counter;;
//            }
//        }
        // если искомое слово найдено, то можно завершать цикл
        if(out!="")
            break;

        --counter;
    }
    file.close();
    return out;
}
