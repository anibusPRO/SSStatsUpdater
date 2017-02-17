#include "gameinforeader.h"
#include <QFile>
#include <QDir>
#include "requestsender.h"
#include <QVariantList>
#include <QCoreApplication>
#include <QSettings>
#include <windows.h>

using namespace Network;

GameInfoReader::GameInfoReader()
{
    _game_info = 0;
    errors_list <<"Stats processed without errors"<<
                  "Could not open 'testStats.lua'"<<
                  "'testStats.lua' is not readable"<<
                  "Stats file does not contain 'GSGameStats'"<<
                  "GSGameStats does not contain 'WinBy'"<<
                  "GSGameStats does not contain 'Duration'"<<
                  "Current game is still in progress"<<
                  "GSGameStats does not contain 'Teams'"<<
                  "Unsupported type of game. The number of teams is not equals two."<<
                  "'userdata' is not exists or is not readable"<<
                  "'userdata' folder is empty"<<
                  "'account_id32_str' is Null"<<
                  "Steam response does not contain response"<<
                  "Steam response does not contain players"<<
                  "Players list is empty"<<
                  "Steam response does not contain personaname"<<
                  "Could not open 'temp.rec'"<<
                  "GSGameStats does not contain player id"<<
                  "Player does not contain 'PHuman'"<<
                  "Player is not human"<<
                  "Player does not contain players info"<<
                  "Unsupported type of game. Number of players per team is not equal."<<
                  "Game options is not standart"<<
                  "Sender is not player"<<
                  "Winners count less than game type";
    last_playback  = 0;
    last_startgame = 0;
    last_stopgame  = 0;
    playback_error = 0;
    is_playback = false;
}

GameInfoReader::~GameInfoReader()
{
    if(_game_info) delete _game_info;
}
// возвращает true если первое время больше второго иначе false
bool GameInfoReader::timeCompare(QTime t1, QTime t2)
{
    if(t2.hour()==23&&t1.hour()<=3)
        return true;
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
    int minutes = abs(dhm*60 - m);

    return minutes;
}

void GameInfoReader::set_ss_path(const QString &value)
{
    ss_path = value;
}

QString GameInfoReader::get_steam_id()
{
    if(sender_steam_id.isEmpty())
        return names_steamids.values().first();
    else
        return sender_steam_id;
}

void GameInfoReader::set_server_addr(QString addr)
{
    server_addr = addr;
}


int GameInfoReader::readySend()
{
    read_warnings_log("APP -- Game Stop", -1);

//    qDebug() << last_playback << last_startgame << last_stopgame << playback_error;
    if(last_playback>last_stopgame)
    {
        is_playback = true;
        return 2;
    }
    if(is_playback)
    {
        is_playback = false;
        return 3;
    }
    if(last_startgame>last_stopgame)
        return 1;
    return 0;
}

bool GameInfoReader::tempRecExist()
{
    return (last_startgame > playback_error);
}


QString GameInfoReader::get_cur_profile_dir()
{
    QDir temp_dir(ss_path+"/Profiles");

    QString name;
    name = read_warnings_log("Using player profile", 3, 5);
    int n=0;
    while(n<5&&name=="")
    {
        name = read_warnings_log("Using player profile", 3, 5);
        Sleep(5000);
        n++;
    }
    QString profile = "Profile1";

    // если имя профиля не изменилось, то отправим пустую строку, как знак того что профиль не сменился
    if(name.isEmpty())
        return QString(ss_path +"/Profiles/"+ profile);
    if(name!=cur_profile_name)
        qDebug() << "cur profile name:" << name << cur_profile_name;

    QStringList profiles = temp_dir.entryList();
//    foreach (QString p, profiles)
//        qDebug() << p;

    if(!profiles.isEmpty())
        foreach (QString file, profiles)
        {
            if(file!="."&&file!="..")
            {
//                qDebug() << file;
                if(QDir(temp_dir.path()+"/"+file).exists())
                {
                    if(temp_dir.cd(file))
                    {
                        QFile temp(temp_dir.path()+"/name.dat");
                        QString str="";
                        if (temp.open(QIODevice::ReadOnly))
                        {
                            QDataStream in(&temp);
                            int filesize = temp.size()-2;
                            char *bytes = new char[filesize];
                            in.device()->seek(2);
                            in.readRawData(bytes, filesize);

                            str = QString::fromUtf16((ushort*)bytes).left(filesize/2);


                            temp.close();
                            delete[] bytes;
                        }
                        else
                        {
                            qDebug() << temp_dir.path() << file;
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
                        else
                            qDebug() << name << str;
                        temp_dir.cdUp();
                    }
                    else
                        qDebug() << "debug 2";
                }
                else
                    qDebug() << "debug 1";
            }
        }
//    qDebug() << profile;
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
void GameInfoReader::setTotalActions(long n)
{
    TotalActions = n;
}
QString GameInfoReader::get_url(QString profile, QString path_to_playback)
{
    int e_code = get_game_info(profile, path_to_playback);
    if(e_code<errors_list.size())
        qDebug() << errors_list.at(e_code);
    else
        qDebug() << "error code is da biggest";
    QString url;
    if(e_code==0)
        url = _game_info->get_url(server_addr+"/connect.php?");

    if(e_code>=5||e_code==0)
        delete _game_info;
    return url;
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
            double AverageAPM = (double)TotalActions*60.0f/(double)duration;
            qDebug() << "AverageAPM with testStats duration:" << AverageAPM;
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
                // иначе запишим условие победы как дисконнект
                _game_info->set_winby("disconnect");

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

            if(names_steamids.isEmpty())
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

            // если настройки игры стандартные, то отправим статистику
            int team_1_p_count=0, team_2_p_count=0, first_team_id=0;
            int fnl_state, sender_id, winners_count=0;
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
                                if(names_steamids.keys().contains(p_name))
                                {
                                    _game_info->set_sender_name(p_name);
//                                    _game_info->setAPMR(average_apm);
                                    _game_info->setAPMR(AverageAPM);
                                    if(sender_steam_id.isEmpty())
                                        sender_steam_id = names_steamids.value(p_name);
//                                    steam_id64.at(sender_names.indexOf(p_name));
                                    _game_info->set_steam_id(sender_steam_id);
                                    fnl_state = player.value("PFnlState").toInt();
                                    sender_id = i;
                                    sender_is_player = true;
                                }
                                else
                                    fnl_state = player.value("PFnlState").toInt();
                                if(fnl_state==5)
                                    winners_count++;
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

            if(team_1_p_count==team_2_p_count)
                _game_info->set_type(team_1_p_count);
            else
                return 21;


            if(_game_info->get_winby()=="disconnect")
            {
                qDebug() << "WinBy:" << "DISCONNECT";
                int sender_team = _game_info->getPlayer(sender_id).team;
                int loseleavers=1;
                for(int i=0; i<players_count; ++i)
                    if((i!=sender_id&&_game_info->getPlayer(i).team == sender_team)&&
                            (_game_info->getPlayer(i).fnl_state!=1))
                                ++loseleavers;
                if(loseleavers==team_1_p_count)
                {
                    _game_info->set_winby("ANNIHILATE");
                    for(int i=0; i<players_count; ++i)
                        if(_game_info->getPlayer(i).team != sender_team)
                        {
                            _game_info->update_player(i, 5);
                            winners_count++;
                        }
                }
            }
            if(winners_count!=team_1_p_count)
                return 24;

            _playback.clear();

            if(last_startgame > playback_error)
            {
                RepReader rep_reader;
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
                qDebug() << "AverageAPM with replay duration:" << (double)TotalActions*60.0f/(double)rep_reader.replay->Duration;

                // переименовываем название реплея в игре по стандарту
                rep_reader.RenameReplay();
                pfile.seek(0);
                _playback = pfile.readAll();

                pfile.close();

                if(!rep_reader.isStandart(team_1_p_count))
                    return 22;

                if(rep_reader.playerIsObserver(_game_info->get_sender_name()))
                    return 23;
            }
            else
                qDebug() << "REC -- Error opening file playback:temp.rec for write";
        }
        else
            return 4;
    }
    else
        return 3;

    return 0;
}

bool GameInfoReader::init_player()
{
    RequestSender sender;
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
    QString reg_url = server_addr+"/regplayer.php?&key="+QLatin1String(SERVER_KEY)+"&";
    int i=0;
    if(userdata_dir.cd(steam_path + "/userdata"))
    {
        if(userdata_dir.entryInfoList().size()>2)
        {
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
                        QByteArray steam_response = sender.getWhileSuccess(request);
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
                                        names_steamids.insert(player_name, account_id64_str);
                                        qDebug() << "Steam ID associated with Soulstorm:" << account_id64_str << player_name;
                                        QString hex_name(player_name.toUtf8().toHex());
                                        QString num = QString::number(i);
                                        reg_url += "name"+num+"="+hex_name+"&sid"+num+"="+account_id64_str+"&";
                                        ++i;
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

    if(names_steamids.isEmpty()||error_code!=0)
    {
        qDebug() << errors_list.at(error_code);
        return false;
    }
    qDebug() << reg_url;
    Request request_to_server(reg_url);
    QByteArray response_fromstat = sender.getWhileSuccess(request_to_server);
    qDebug() << "registration in dowstats: " << QString::fromUtf8(response_fromstat.data());
    if(response_fromstat.isNull())
        qDebug() << "Server returned empty data";
    return true;
}

QString GameInfoReader::read_warnings_log(QString str, int offset/*=0*/, int count/*=1*/)
{
    QString out="";
    QFile file(ss_path+"/warnings.log");

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open warnings.log";
        return QString();
    }

    QTextStream textStream(&file);

    QStringList game_flags;
    game_flags << "APP -- Game Stop"
               << "APP -- Game Start"
               << "APP -- Game Playback"
               << "REC -- Error opening file playback:temp.rec for write";


    bool fstate = false;
    QStringList fileLines = textStream.readAll().split("\n");
    int counter = fileLines.size();
    while (counter!=0)
    {
        // читаем следующую строку из файла
        QString line = fileLines.at(counter-1);
        // если текущаю строка содержит переданную в параметрах строку
        if(line.contains(str/*, Qt::CaseInsensitive*/))
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
            if(str.contains(" "))
                // делим переданную строку через пробел, получаем первый элемент списка
                // к примеру если передали Game Start, то получим Game
                first_word = str.split(" ")[0];
            // если в списке полученном из текущей строки есть искомое слово
            if(list.contains(first_word/*, Qt::CaseInsensitive*/))
                // пишем в вывод строку из списка с отступом, переданном в параметре
                if(list.indexOf(first_word/*, Qt::CaseInsensitive*/)!=-1)
                    for(int i=list.indexOf(first_word)+offset, j=0; i<list.size()&&j<count; ++i, ++j)
                        out += list[i] + " ";
                else
                    qDebug() << "out of range" << first_word << str << counter;
            if(out!="")
                out = out.left(out.size()-1);

        }
        line = line.remove("\n");
        line = line.remove("\r");
        if(!fstate&&game_flags.contains(line.mid(14, line.size()-3)))
        {
            if(line.contains(game_flags.at(0)))
            {
                last_stopgame = counter;
                fstate = true;
            }
            if(line.contains(game_flags.at(1)))
                last_startgame = counter;
            if(line.contains(game_flags.at(2)))
                last_playback = counter;
            if(line.contains(game_flags.at(3)))
                playback_error = counter;
        }

        // если искомое слово найдено, то можно завершать цикл
        if(out!="")
            break;
        --counter;
    }
    file.close();
    return out;
}
