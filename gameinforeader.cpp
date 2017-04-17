#include "gameinforeader.h"
#include <QFile>
#include <QDir>
#include "requestsender.h"
#include <QVariantList>
#include <QCoreApplication>
#include <QSettings>
#include <windows.h>

GameInfoReader::GameInfoReader()
{
    errors_list <<"SUCCESS! Stats processed without errors"<<                                      /* 0*/
                  "ERROR! Could not open 'testStats.lua'"<<                                      /* 1*/
                  "ERROR! 'testStats.lua' is not readable"<<                                     /* 2*/
                  "ERROR! Stats file does not contain 'GSGameStats'"<<                           /* 3*/
                  "ERROR! GSGameStats does not contain 'WinBy'"<<                                /* 4*/
                  "ERROR! GSGameStats does not contain 'Duration'"<<                             /* 5*/
                  "ERROR! GSGameStats does not contain Players"<<                                /* 6*/
                  "ERROR! GSGameStats does not contain Scenario"<<                               /* 7*/
                  "ERROR! GSGameStats does not contain 'Teams'"<<                                /* 8*/
                  "ERROR! Unsupported type of game. The number of teams is not equals two."<<    /* 9*/
                  "ERROR! Player left the game"<<                                   /*10*/
                  "ERROR! GSGameStats does not contain player id"<<                              /*11*/
                  "ERROR! Player does not contain 'PHuman'"<<                                    /*12*/
                  "ERROR! Player is not human"<<                                                 /*13*/
                  "ERROR! Player does not contain players info"<<                                /*14*/
                  "ERROR! Unsupported type of game. Number of players per team is not equal."<<  /*15*/
                  "ERROR! Could not open 'temp.rec'"<<                                           /*16*/
                  "ERROR! Game options is not standart";                                         /*17*/
//                  "'userdata' is not exists or is not readable"<<                         /* 9*/
//                  "'userdata' folder is empty"<<                                          /*10*/
//                  "'account_id32_str' is Null"<<                                          /*11*/
//                  "Steam response does not contain response"<<                            /*12*/
//                  "Steam response does not contain players"<<                             /*13*/
//                  "Players list is empty"<<                                               /*14*/
//                  "Steam response does not contain personaname"<<                         /*15*/
//                  "Sender is not player"<<
//                  "Winners count less than game type"<<                                   /*26*/
    last_playback  = 0;
    last_startgame = 0;
    last_stopgame  = 0;
    last_ending_mission = 0;
    playback_error = 0;
    _game_info = 0;
    is_playback = false;
    profile_error = true;
}

GameInfoReader::~GameInfoReader()
{
    if(_game_info) delete _game_info;
}

void GameInfoReader::set_ss_path(const QString &value)
{
    ss_path = value;
}

void GameInfoReader::set_account(QString SID, QString name)
{
    sender_steamID = SID;
    sender_name = name;
}

QString GameInfoReader::get_steam_id() const
{
    return sender_steamID;
}

int GameInfoReader::readySend()
{
//    read_warnings_log("GAME -- Ending mission", -1);
    read_warnings_log("APP -- Game Stop", -1);

//    qDebug() << last_playback << last_startgame << last_stopgame << playback_error << last_ending_mission;
    if(last_playback>last_stopgame&&last_playback>last_ending_mission)
    {
        is_playback = true;
        return 2;
    }
    if(is_playback)
    {
        is_playback = false;
        return 3;
    }
    if(last_startgame>last_stopgame&&last_startgame>last_ending_mission)
        return 1;
    return 0;
}

QString GameInfoReader::get_cur_profile_dir()
{
    QDir temp_dir(ss_path+"/Profiles");
    QString name;
    for(int i=0; i<3&&name.isEmpty();++i)
    {
        name = read_warnings_log("Using player profile", 3, 5);
        Sleep(3000);
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
                            if(!profile_error)
                            {
                                qDebug() << temp_dir.path();
                                qDebug() << "Could not open name.dat";
                                profile_error = false;
                            }
                            temp_dir.cdUp();
                            continue;
                        }
                        if(name == str)
                        {
                            profile = file;
                            cur_profile_name = str;
                            break;
                        }
                        else if(!profile_error)
                        {
                            qDebug() << name << str;
                            profile_error = false;
                        }
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

bool GameInfoReader::is_map_valid()
{
    QString map_name;
    QString error_str = read_warnings_log("Lobby -- Setting up game with scenario", 7, 5);
    if(error_str.contains("(invalid)"))
    {
        map_name = error_str.remove(" (invalid)");
        if(last_invalid_map==map_name)
            return true;
        last_invalid_map = map_name;
        if(QFile::exists(ss_path + "/DXP2/Data/Scenarios/mp/" + map_name+".sgb"))
            qDebug() << "map" << map_name << "is invalid";
        else
            qDebug() << "map does not exist" << map_name;
        return false;
    }

    return true;
}

QByteArray GameInfoReader::get_playback_file()
{
    return _playback;
}
QString GameInfoReader::get_playback_name() const
{
    return playback_name;
}

void GameInfoReader::setTotalActions(long n)
{
    TotalActions = n;
}
QString GameInfoReader::get_game_info(QString profile)
{
    int e_code = search_info(profile);
    if(e_code<errors_list.size())
        qDebug() << errors_list.at(e_code);
    else
        qDebug() << "error code is da biggest";
    QString info;
    if(e_code==0)
        info = _game_info->get_params();
    if(e_code>10) delete _game_info;
    return info;
}

int GameInfoReader::search_info(QString profile)
{
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
    QByteArray testStats = file.readAll();
    QVariantMap stats = QtJson::to_map(testStats);
    file.close();
    int parse_res = 0;
    if(!stats.contains("GSGameStats")) parse_res = 3;
    QVariantMap GSGameStats = stats.value("GSGameStats").toMap();

    if(!GSGameStats.contains("WinBy"    )) parse_res = 4;
    if(!GSGameStats.contains("Duration" )) parse_res = 5;
    if(!GSGameStats.contains("Players"  )) parse_res = 6;
    if(!GSGameStats.contains("Scenario" )) parse_res = 7;
    if(!GSGameStats.contains("Teams"    )) parse_res = 8;
    if(parse_res!=0){
        qDebug() << testStats;
        return parse_res;
    }
    QString winby = GSGameStats.value("WinBy").toString();    // условие победы
    int players_count = GSGameStats.value("Players").toInt(); // количество игроков
    int duration = GSGameStats.value("Duration").toInt();     // продолжительность игры
    int team_num = GSGameStats.value("Teams").toInt();        // количество команд

    if(team_num!=2) return 9;
    if(duration<MINIMUM_DURATION) return 10;

    double AverageAPM = (double)TotalActions*60.0f/(double)duration;
    qDebug() << "WinBy:" << winby.toUpper();
    qDebug() << "AverageAPM with testStats duration:" << AverageAPM;

    _game_info = new GameInfo(players_count);

    // если настройки игры стандартные, то отправим статистику
    int teams[8] = {0};
    int fnl_state, sender_id=-1, winners_count=0;
    QString p_name;
//    bool sender_is_player = false;
    for(int i=0; i<players_count;++i)
    {
        QString player_id = "player_"+QString::number(i);
        if(!GSGameStats.contains(player_id)) return 11;
        QVariantMap player = GSGameStats.value(player_id).toMap();
        if(!player.contains("PHuman")) return 12;
        if(player.value("PHuman").toInt()!=1)
            return 13;
        if(!(player.contains("PName")&&player.contains("PRace")
                &&player.contains("PFnlState")&&player.contains("PTeam")))
            return 14;

        teams[player.value("PTeam").toInt()]++;

        p_name = player.value("PName").toString();
        if(sender_name==p_name)
        {
            _game_info->set_sender_name(p_name);
            fnl_state = player.value("PFnlState").toInt();
            sender_id = i;
        }
        else
            fnl_state = player.value("PFnlState").toInt();

        if(fnl_state==5) winners_count++;
        _game_info->add_player(p_name, player.value("PRace").toString(),
                                   player.value("PTeam").toInt(), fnl_state/*, rep_reader.GetAverageAPM(i)*/);
    }

    if(teams[0]!=teams[1]) return 15;
    int game_type = teams[0];
    if(sender_id!=-1&&winby.isEmpty())
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
    // раньше это условие срабартывало когда игрок ливал и мы не отправляли статистику,
    // теперь же, статистика будет отправлять даже в тако случе
//    if(winners_count!=game_type) return 24;
    _game_info->set_winby(winby.isEmpty()?"disconnect":winby);
    _playback.clear();
    if(last_startgame > playback_error)
    {
        RepReader rep_reader;
        qDebug() << "playback reading";
        // откроем реплей файл последней игры для чтения
        QFile pfile(ss_path+"/Playback/temp.rec");
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
            return 17;
        // теперь, если игрок обозреватель, то пусть отправляет статистику как ливер
        // все таки обозреватель может ливнуть в любой момент и реплей может быть не полным
        // но зато хоть какой-то реплей будет
        if(rep_reader.playerIsObserver(_game_info->get_sender_name()))
//            return 23;
            _game_info->set_winby("disconnect");
    }
    else
        qDebug() << "REC -- Error opening file playback:temp.rec for write";

    QString mod_name = read_warnings_log("MOD -- Initializing Mod", 4);  // имя запущенного мода

    _game_info->set_steam_id(sender_steamID);
    _game_info->set_APMR(AverageAPM);
    _game_info->set_type(game_type);
    _game_info->set_team_number(team_num);
    _game_info->set_duration(duration);
    _game_info->set_map_name(GSGameStats.value("Scenario").toString());
    _game_info->set_mod_name(mod_name);

    return 0;
}

QStringList GameInfoReader::get_players(QString profile)
{
    QFile file(profile + "/testStats.lua");
    if(!file.open(QIODevice::ReadOnly))
        return QStringList();
    if(!file.isReadable())
        return QStringList();
    int k=0;
    while(file.read(1)!="G")
        k++;
    file.seek(k);
    QByteArray testStats = file.readAll();
    QVariantMap stats = QtJson::to_map(testStats);
    file.close();
    int parse_res = 0;
    if(!stats.contains("GSGameStats")) parse_res = 3;
    QVariantMap GSGameStats = stats.value("GSGameStats").toMap();
    if(!GSGameStats.contains("Players")) parse_res = 6;
    if(parse_res!=0){
        qDebug() << errors_list.at(parse_res);
        qDebug() << testStats;
        return QStringList();
    }
    int players_count = GSGameStats.value("Players").toInt(); // количество игроков

    QStringList result;
//    result.reserve(8);
    QList<QVariantMap> players;
    int sender_team, sender_id=-1;
    for(int i=0; i<players_count;++i)
    {
        QString player_id = "player_"+QString::number(i);
        if(!GSGameStats.contains(player_id)) continue;
        QVariantMap player = GSGameStats.value(player_id).toMap();
        if(!(player.contains("PName")&&player.contains("PRace")&&player.contains("PTeam")))
            continue;
        if(sender_name==player.value("PName").toString())
        {
            sender_team = player.value("PTeam").toInt();
            sender_id = i;
        }
        players << player;
    }
    if(sender_id!=-1)
        for(int i=0; i<players.size();++i)
        {
            if(players.at(i).value("PTeam")!=sender_team)
                result << QString(players.at(i).value("PName").toString()+" - "+
                                   players.at(i).value("PRace").toString());
            else
                result << QStringList();
//            QString(players.at(i).value("PName").toString()+" - "+

        }
    else
        return QStringList();
    return result;
}


QString GameInfoReader::get_last_invalid_map() const
{
    return last_invalid_map;
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
               << "REC -- Error opening file playback:temp.rec for write"
               << "GAME -- Ending mission";


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
            if(list.contains(first_word)){
                // пишем в вывод строку из списка с отступом, переданном в параметре
                if(list.indexOf(first_word)!=-1)
                    for(int i=list.indexOf(first_word)+offset, j=0; i<list.size()&&j<count; ++i, ++j)
                        out += list[i] + " ";
                else
                    qDebug() << "out of range" << first_word << str << counter;
            }
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
            if(line.contains(game_flags.at(4)))
                last_ending_mission = counter;
        }

        // если искомое слово найдено, то можно завершать цикл
        if(out!="")
            break;
        --counter;
    }
    file.close();
    return out;
}
