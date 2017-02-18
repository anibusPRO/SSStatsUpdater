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
    errors_list <<"Stats processed without errors"<<                                      /* 0*/
                  "Could not open 'testStats.lua'"<<                                      /* 1*/
                  "'testStats.lua' is not readable"<<                                     /* 2*/
                  "Stats file does not contain 'GSGameStats'"<<                           /* 3*/
                  "GSGameStats does not contain 'WinBy'"<<                                /* 4*/
                  "GSGameStats does not contain 'Duration'"<<                             /* 5*/
                  "Current game is still in progress"<<                                   /* 6*/
                  "GSGameStats does not contain 'Teams'"<<                                /* 7*/
                  "Unsupported type of game. The number of teams is not equals two."<<    /* 8*/
                  "'userdata' is not exists or is not readable"<<                         /* 9*/
                  "'userdata' folder is empty"<<                                          /*10*/
                  "'account_id32_str' is Null"<<                                          /*11*/
                  "Steam response does not contain response"<<                            /*12*/
                  "Steam response does not contain players"<<                             /*13*/
                  "Players list is empty"<<                                               /*14*/
                  "Steam response does not contain personaname"<<                         /*15*/
                  "Could not open 'temp.rec'"<<                                           /*16*/
                  "GSGameStats does not contain player id"<<                              /*17*/
                  "Player does not contain 'PHuman'"<<                                    /*18*/
                  "Player is not human"<<                                                 /*19*/
                  "Player does not contain players info"<<                                /*20*/
                  "Unsupported type of game. Number of players per team is not equal."<<  /*21*/
                  "Game options is not standart"<<                                        /*22*/
                  "Sender is not player"<<                                                /*23*/
                  "Winners count less than game type"<<                                   /*24*/
                  "GSGameStats does not contain Players"<<                                 /*25*/
                  "GSGameStats does not contain Scenario";                                /*26*/
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

    if(!profiles.isEmpty())
        foreach (QString file, profiles)
        {
            if(file!="."&&file!="..")
            {
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
    return QString(ss_path +"/Profiles/"+ profile);

}

QByteArray GameInfoReader::get_playback_file()
{
    return _playback;
}
QString GameInfoReader::get_playback_name()
{
    return playback_name;
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
    if(names_steamids.isEmpty()) return error_code;

    QFile file(profile + "/testStats.lua");

    if(!file.open(QIODevice::ReadOnly))
        return 1;
    if(!file.isReadable())
        return 2;
    // в начале файла лежат байты из-за этого корневой ключ может не читаться
    int k=0;
    while(file.read(1)!="G")
        k++;
    file.seek(k);
    QVariantMap stats = QtJson::to_map(file.readAll());
    file.close();

    if(!stats.contains("GSGameStats")) return 3;
    QVariantMap GSGameStats = stats.value("GSGameStats").toMap();

    if(!GSGameStats.contains("WinBy")) return 4;
    if(!GSGameStats.contains("Players")) return 25;
    if(!GSGameStats.contains("Duration")) return 5;
    if(!GSGameStats.contains("Scenario")) return 26;
    if(!GSGameStats.contains("Teams")) return 7;

    QString winby = GSGameStats.value("WinBy").toString();    // условие победы
    int players_count = GSGameStats.value("Players").toInt(); // количество игроков
    int duration = GSGameStats.value("Duration").toInt();     // продолжительность игры
    int team_num = GSGameStats.value("Teams").toInt();

    if(team_num!=2) return 8;
    if(duration<MINIMUM_DURATION) return 6;

    double AverageAPM = (double)TotalActions*60.0f/(double)duration;
    qDebug() << "WinBy:" << winby.toUpper();
    qDebug() << "AverageAPM with testStats duration:" << AverageAPM;

    _game_info = new GameInfo(players_count);

    // если настройки игры стандартные, то отправим статистику
    int game_type=players_count/2, teams[8] = {0};
    int fnl_state, sender_id, winners_count=0;
    QString p_name;
    bool sender_is_player = false;
    for(int i=0; i<players_count;++i)
    {
        QString player_id = "player_"+QString::number(i);
        if(!GSGameStats.contains(player_id))
        {
            qDebug() << "GSGameStats does not contain" << player_id;
            return 17;
        }
        QVariantMap player = GSGameStats.value(player_id).toMap();
        if(!player.contains("PHuman"))
            return 18;
//        if(player.value("PHuman").toInt()!=1)
//            return 19;
        if(!(player.contains("PName")&&player.contains("PRace")
                &&player.contains("PFnlState")&&player.contains("PTeam")))
            return 20;

        teams[player.value("PTeam").toInt()]++;

        p_name = player.value("PName").toString();
        if(names_steamids.keys().contains(p_name))
        {
            _game_info->set_sender_name(p_name);
            _game_info->setAPMR(AverageAPM);
            if(sender_steam_id.isEmpty())
                sender_steam_id = names_steamids.value(p_name);
            _game_info->set_steam_id(sender_steam_id);
            fnl_state = player.value("PFnlState").toInt();
            sender_id = i;
            sender_is_player = true;
        }
        else
            fnl_state = player.value("PFnlState").toInt();

        if(fnl_state==5) winners_count++;
        _game_info->add_player(p_name, player.value("PRace").toString(),
                                   player.value("PTeam").toInt(), fnl_state/*, rep_reader.GetAverageAPM(i)*/);
    }

    if(!sender_is_player) return 23;
    // если количество игроков в первой команде равно количеству игроков во второй команде,
    // то тип игры равен количеству игроков в одной из команд
//    qDebug() << teams[0] << teams[1];
    if(teams[0]!=teams[1]) return 21;

    if(winby.isEmpty())
    {
        int sender_team = _game_info->getPlayer(sender_id).team;
        int loseleavers=1;
        for(int i=0; i<players_count; ++i)
            if((i!=sender_id&&_game_info->getPlayer(i).team == sender_team)&&
                    (_game_info->getPlayer(i).fnl_state!=1))
                        ++loseleavers;
        if(loseleavers==game_type)
        {
            winby = "ANNIHILATE";
            for(int i=0; i<players_count; ++i)
                if(_game_info->getPlayer(i).team != sender_team)
                {
                    _game_info->update_player(i, 5);
                    winners_count++;
                }
        }
    }
    if(winners_count!=game_type) return 24;

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

        // переименовываем название реплея в игре по стандарту
        playback_name = rep_reader.RenameReplay()+".rec";
        qDebug() << playback_name;
        pfile.seek(0);
        _playback = pfile.readAll();

        pfile.close();

        if(!rep_reader.isStandart(game_type))
            return 22;

        if(rep_reader.playerIsObserver(_game_info->get_sender_name()))
            return 23;
    }
    else
        qDebug() << "REC -- Error opening file playback:temp.rec for write";

    QString mod_name = read_warnings_log("MOD -- Initializing Mod", 4);  // имя запущенного мода

    _game_info->set_type(game_type);
    _game_info->set_team_number(team_num);
    _game_info->set_duration(duration);
    _game_info->set_map_name(GSGameStats.value("Scenario").toString());
    _game_info->set_mod_name(mod_name);
    _game_info->set_winby(winby.isEmpty()?"disconnect":winby);

    return 0;
}

bool GameInfoReader::init_player()
{
    RequestSender sender;
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
    {
        qDebug() << errors_list.at(9);
        return false;
    }

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
                    }

                }

            }
            userdata_dir.cdUp();
        }
    }

    if(names_steamids.isEmpty()) return false;

    qDebug() << reg_url;
    Request request_to_server(reg_url);
    QByteArray response_fromstat = sender.getWhileSuccess(request_to_server);
    qDebug() << "registration in dowstats: " << QString::fromUtf8(response_fromstat.data());

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
            if(list.contains(first_word))
                // пишем в вывод строку из списка с отступом, переданном в параметре
                if(list.indexOf(first_word)!=-1)
                    for(int i=list.indexOf(first_word)+offset, j=0; i<list.size()&&j<count; ++i, ++j)
                        out += list[i] + " ";
                else
                    qDebug() << "out of range" << first_word << str << counter;
            if(out!="")
                out = out.left(out.size()-1);

        }
        if(!fstate)
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
