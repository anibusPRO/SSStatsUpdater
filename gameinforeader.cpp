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
/* 0*/    errors_list <<"SUCCESS! Stats processed without errors"<<
/* 1*/                  "ERROR! Could not open 'testStats.lua'"<<
/* 2*/                  "ERROR! 'testStats.lua' is not readable"<<
/* 3*/                  "ERROR! Stats file does not contain 'GSGameStats'"<<
/* 4*/                  "ERROR! GSGameStats does not contain 'WinBy'"<<
/* 5*/                  "ERROR! GSGameStats does not contain 'Duration'"<<
/* 6*/                  "ERROR! GSGameStats does not contain Players"<<
/* 7*/                  "ERROR! GSGameStats does not contain Scenario"<<
/* 8*/                  "ERROR! GSGameStats does not contain 'Teams'"<<
/* 9*/                  "ERROR! Unsupported type of game. The number of teams is not equals two."<<
/*10*/                  "ERROR! Player left the game"<<
/*11*/                  "ERROR! GSGameStats does not contain player id"<<
/*12*/                  "ERROR! Player does not contain 'PHuman'"<<
/*13*/                  "ERROR! Player is not human"<<
/*14*/                  "ERROR! Player does not contain players info"<<
/*15*/                  "ERROR! Unsupported type of game. Number of players per team is not equal."<<
/*16*/                  "ERROR! Could not open 'temp.rec'"<<
/*17*/                  "ERROR! Game options is not standart"<<
/*18*/                  "Current game is playback";
    is_playback = false;
    soulstorm_start_dt = QDateTime();
    current_mod_index = 0;
    last_playback  = 0;
    last_startgame = 0;
    last_stopgame  = 0;
    playback_error = 0;
}

GameInfoReader::~GameInfoReader()
{
}

void GameInfoReader::init(QString path, QString SID, QString name)
{
    ss_path = path;
    sender_steamID = SID;
    sender_name = name;
}

void GameInfoReader::reset()
{
    is_playback = false;
    current_mod_index = 0;
    last_playback  = 0;
    last_startgame = 0;
    last_stopgame  = 0;
    playback_error = 0;
}

int GameInfoReader::readySend(bool debug)
{
    read_warnings_log("APP -- Game Stop", -1);
    if(debug)
        qDebug() << last_startgame << last_stopgame << last_playback << playback_error;

    if(last_startgame==last_stopgame)
        return 3;

    if(last_playback>last_stopgame)
    {
        is_playback = true;
        return 2;
    }

    if(last_startgame>last_stopgame)
        return 1;

    return 0;
}

QString GameInfoReader::getSender_name() const
{
    return sender_name;
}

void GameInfoReader::setSender_name(const QString &value)
{
    sender_name = value;
}

QString GameInfoReader::find_profile()
{
    QDir profiles_dir(ss_path+"/Profiles");
    QString profile_dir;
    QDateTime tempDT;
    foreach (QString profile, profiles_dir.entryList(QDir::Dirs))
    {
        if(!profile.contains(QRegExp("Profile[0-9]{1,2}", Qt::CaseInsensitive)))
            continue;
        if(!QFile::exists(ss_path+"/Profiles/"+profile+"/testStats.Lua"))
            continue;
        QDateTime fileDT = QFileInfo(ss_path+"/Profiles/"+profile+"/testStats.Lua").lastModified();
        if(tempDT<fileDT)
        {
            profile_dir = profile;
            tempDT = fileDT;
        }
    }
    if(!profile_dir.isEmpty())
        return QString(ss_path+"\\Profiles\\"+profile_dir);

    QSettings settings("Local.ini", QSettings::IniFormat);
    QString LocalINIProfile = settings.value("global/playerprofile", "Profile1").toString();

    return QString(ss_path+"\\Profiles\\"+LocalINIProfile);

//    QDir profiles_dir(ss_path+"/Profiles");
//    QString name;
//    for(int i=0; i<3&&name.isEmpty();++i)
//    {
//        name = read_warnings_log("Using player profile", 3, 5);
//        Sleep(3000);
//    }

//    QSettings settings("Local.ini", QSettings::IniFormat);
//    QString LocalINIProfile = settings.value("global/playerprofile", "Profile1").toString();

//    if(name.isEmpty())
//        return QString(ss_path+"/Profiles/"+LocalINIProfile);

//    QFile nameDat(ss_path+"/Profiles/"+LocalINIProfile+"/name.dat");
//    if (nameDat.exists()&&nameDat.open(QIODevice::ReadOnly))
//    {
//        QString str = QString::fromUtf16((ushort*)nameDat.readLine().data());
//        nameDat.close();
//        if(name == str)
//            return QString(ss_path+"/Profiles/"+LocalINIProfile);
//    }

//    foreach (QString profile_dir, profiles_dir.entryList(QDir::Dirs))
//    {
//        if(!profile_dir.contains(QRegExp("Profile[0-9]{1,2}")))
//            continue;
//        QFile nameDat(profiles_dir.path()+"/"+profile_dir+"/name.dat");
//        QString str="";
//        if (nameDat.exists()&&nameDat.open(QIODevice::ReadOnly))
//        {
//            str = QString::fromUtf16((ushort*)nameDat.readLine().data());
//            nameDat.close();
//        }
//        else
//        {
//            qDebug() << "Could not open" << nameDat.fileName();
//            continue;
//        }
//        if(name == str)
//            return QString(ss_path+"/Profiles/"+profile_dir);
//        else
//            qDebug() << "warnings.log profile:" << name << QString(profile_dir+"/name.dat profile:") << str;
//    }
//    return QString(ss_path+"/Profiles/"+LocalINIProfile);
}

QString GameInfoReader::get_cur_profile_dir()
{
    QString profile = find_profile();
    if(cur_profile_folder!=profile)
    {
        cur_profile_folder = profile;
        qDebug() << "Profile:" << cur_profile_folder;
    }
    return cur_profile_folder;
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
QString GameInfoReader::get_game_stats() const
{
    return game_stats;
}

int GameInfoReader::read_game_info(QMap<QString, QString> *sids, long totalActions)
{
    if(is_playback)
    {
        is_playback = false;
        return 18;
    }
    QFile file(cur_profile_folder + "/testStats.lua");

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

    double AverageAPM = (double)totalActions*60.0f/(double)duration;
    qDebug() << "WinBy:" << winby.toUpper();
    qDebug() << "AverageAPM with testStats duration:" << AverageAPM;

    GameInfo game_info(players_count);

    // если настройки игры стандартные, то отправим статистику
    int teams[8] = {0};
    int fnl_state, sender_id=-1;
    QString p_name;
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
            fnl_state = player.value("PFnlState").toInt();
            sender_id = i;
        }
        else
            fnl_state = player.value("PFnlState").toInt();
        if(!sids->values().contains(p_name)){
            qDebug() << "sids not contains" << p_name;
            for(auto e : sids->keys()){
              qDebug() << e << sids->value(e);
            }
        }
        game_info.add_player(p_name,
                               GameInfo::races.indexOf(player.value("PRace").toString()),
                               player.value("PTeam").toInt(),
                               fnl_state,
                               sids->key(p_name));
    }

    if(teams[0]!=teams[1]) return 15;
    int game_type = teams[0];
    if(sender_id!=-1&&winby.isEmpty())
    {
        int sender_team = game_info.getPlayer(sender_id).team;
        int loseleavers=1;
        for(int i=0; i<players_count; ++i)
            if((i!=sender_id&&game_info.getPlayer(i).team == sender_team)&&
                    (game_info.getPlayer(i).fnl_state!=1))
                        ++loseleavers;

        if(loseleavers==game_type)
            winby = "ANNIHILATE";

        for(int i=0; i<players_count; ++i)
            if(game_info.getPlayer(i).team != sender_team)
                game_info.update_player(i, 5);
    }

    game_info.set_winby(winby.isEmpty()?"disconnect":winby);
    _playback.clear();
    QString mod_name;
    if(last_startgame >= playback_error)
    {
        qDebug() << "playback reading";
        RepReader rep_reader(ss_path+"/Playback/temp.rec");
        if(!rep_reader.isStandart(game_type))
            return 17;
        // конвертируем реплей в стимовскую версию
        rep_reader.convertReplayToSteamVersion();

        mod_name = rep_reader.replay.MOD;

        QString new_name = rep_reader.RenameReplay();
        // переименовываем название реплея в игре по стандарту
        if(!new_name.isEmpty())
            playback_name = new_name+".rec";
        else
            qDebug() << "Could not change playback name";

        _playback = rep_reader.getReplayData();
    }
    else
    {
        qDebug() << "REC -- Error opening file playback:temp.rec for write";
        // если имя мода не удалось получить из реплея, то получим его из warnings.log
        mod_name = read_warnings_log("MOD -- Initializing Mod", 4);  // имя запущенного мода
    }

    game_info.set_sender_name(sender_name);
    game_info.set_steam_id(sender_steamID);
    game_info.set_APMR(AverageAPM);
    game_info.set_type(game_type);
    game_info.set_team_number(team_num);
    game_info.set_duration(duration);
    game_info.set_map_name(GSGameStats.value("Scenario").toString());
    game_info.set_mod_name(mod_name);

    game_stats = game_info.get_stats();

    return 0;
}

bool GameInfoReader::is_mod_changed()
{
    int index = read_warnings_log("MOD -- Shutting down Mod");
    if(index>current_mod_index)
    {
        current_mod_index = index;
        return true;
    }

//    QString mod_name = read_warnings_log("MOD -- Initializing Mod", 4);
//    if(current_mod != mod_name)
//    {
//        qDebug() << "Mode was changed from" << current_mod << "to" << mod_name;
//        current_mod = mod_name;
//        return true;
//    }
    return false;
}

bool GameInfoReader::is_playback_error()
{
    return last_startgame<playback_error;
}

void GameInfoReader::get_error_debug(int e_code)
{
    if(e_code<errors_list.size())
        qDebug() << errors_list.at(e_code);
    else
        qDebug() << "unknown error code " << e_code;
}

QStringList GameInfoReader::get_players(bool with_mates)
{
    QStringList result;
    QFile file(cur_profile_folder + "/testStats.lua");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "testStats open error";
        return result;
    }
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
        qDebug() << "testStats parsing error";
        qDebug() << errors_list.at(parse_res);
        qDebug() << testStats;
        return result;
    }
    int players_count = GSGameStats.value("Players").toInt(); // количество игроков

    QList<QVariantMap> players;
    int sender_team=0, sender_id=-1;
    for(int i=0; i<players_count;++i)
    {
        QString player_id = "player_"+QString::number(i);
        if(!GSGameStats.contains(player_id)) continue;
        QVariantMap player = GSGameStats.value(player_id).toMap();
        if(!(player.contains("PName")&&player.contains("PRace")&&player.contains("PTeam")))
            continue;
        QString p_name = player.value("PName").toString();
        if(sender_name==p_name)
        {
            sender_team = player.value("PTeam").toInt();
            sender_id = i;
        }
        players << player;
    }
    // если id отправителя не изменился, то его ник не найден в списке игроков
    // а это означает что он обозреватель, поэтому нужно вернуть пустой список
    if(sender_id!=-1)
    {
        for(int i=0; i<players.size();++i)
            if(with_mates||players.at(i).value("PTeam")!=sender_team)
                result << QString(players.at(i).value("PName").toString()+" - "+
                          GameInfo::racesUC.at(GameInfo::races.indexOf(players.at(i).value("PRace").toString())));
    }
//    else{
//        for(int i=0; i<players.size();++i)
//            result << QString(players.at(i).value("PName").toString()+" - "+
//                      GameInfo::racesUC.at(GameInfo::races.indexOf(players.at(i).value("PRace").toString())));
//    }
    return result;
}

QString GameInfoReader::get_last_invalid_map() const
{
    return last_invalid_map;
}

bool GameInfoReader::is_game_restarted()
{
    QFile file(ss_path+"/warnings.log");
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream textStream(&file);
        QDateTime start_dt = QDateTime::fromString(textStream.readLine().split("at ").back(), "yyyy-MM-dd hh:mm");
        if(soulstorm_start_dt!=start_dt){
            qDebug() << "SOULSTORM started at" << start_dt.toString("yyyy-MM-dd hh:mm");
            soulstorm_start_dt = start_dt;
            return true;
        }
        file.close();
    }
    else
        qDebug() << "is_game_restarted()" << "Could not open warnings.log";
    return false;
}


QString GameInfoReader::read_warnings_log(QString str, int offset, int count/*=1*/)
{
    QString out;
    QFile file(ss_path+"/warnings.log");

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "read_warnings_log 1" << "Could not open warnings.log";
        return QString();
    }

    QTextStream textStream(&file);

    QStringList game_flags;
    game_flags << "APP -- Game Stop"
               << "APP -- Game Start"
               << "APP -- Game Playback"
               << "REC -- Error opening file playback:temp.rec for write"
               << "GAME -- Ending mission"
               << "APP -- Game Load";


    bool fstate = false;
    QStringList fileLines = textStream.readAll().split("\r");
    int counter = fileLines.size();
    while (counter!=0)
    {
        // читаем следующую строку из файла
        QString line = fileLines.at(counter-1);
        // если текущаю строка содержит переданную в параметрах строку
        if(line.contains(str))
        {
            // результат поиска
            out.clear();
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
            if(!out.isEmpty())
                out = out.left(out.size()-1);

        }
        if(!fstate)
        {

            if(line.contains(game_flags.at(0))
                    ||(!is_playback&&line.contains(game_flags.at(4)))
                    ||line.contains(game_flags.at(5)))
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
        if(!out.isEmpty())
            break;
        --counter;
    }
    file.close();
    return out;
}

int GameInfoReader::read_warnings_log(QString str)
{
    QFile file(ss_path+"/warnings.log");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "read_warnings_log 2" << "Could not open warnings.log";
        return 0;
    }

    QTextStream textStream(&file);
    QStringList fileLines = textStream.readAll().split("\r");
    int counter = fileLines.size();
    while (counter!=0)
    {
        // читаем следующую строку из файла
        QString line = fileLines.at(counter-1);
        // если текущаю строка содержит переданную в параметрах строку
        if(line.contains(str))
        {
            file.close();
            return counter;
        }
        --counter;
    }

    return 0;
}
