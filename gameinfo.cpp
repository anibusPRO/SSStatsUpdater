#include "gameinfo.h"
#include <QDebug>
#include <QRegExp>


GameInfo::GameInfo(int players_count)
{
    _players.reserve(players_count);
    races.insert("space_marine_race",1);
    races.insert("chaos_marine_race",2);
    races.insert("ork_race",3);
    races.insert("eldar_race",4);
    races.insert("guard_race",5);
    races.insert("necron_race",6);
    races.insert("tau_race",7);
    races.insert("sisters_race",8);
    races.insert("dark_eldar_race",9);
    _players_count = players_count;
    _map_name = "";
    _type = 0;
    _winby = "";
    _teams_number = 0;
    _duration = 0;
    _sender_name = "";
    _steam_id = "";
}

GameInfo::~GameInfo()
{
}

void GameInfo::set_type(int type)
{
    _type = type;
}

void GameInfo::set_winby(QString str)
{
    _winby = str;
}

void GameInfo::set_duration(int time)
{
    _duration = time;
}

void GameInfo::set_team_number(int num)
{
    _teams_number = num;
}

void GameInfo::set_sender_name(QString name)
{
    _sender_name = name;
}

void GameInfo::set_steam_id(QString id)
{
    _steam_id = id;
}

QString GameInfo::get_url(QString site_addr)
{
    bool sender_is_player = false;

    // добавим имена игроков
    for(int i=0; i<_players_count; ++i)
    {
//        QByteArray btr = _players.at(i).name.toAscii();
//        QString p_name(btr.toHex());
        QString p_name = _players.at(i).name;
        site_addr += "p" + QString::number(i+1) + "="
                + p_name + "&";
        if(!sender_is_player&&_sender_name==_players.at(i).name)
            sender_is_player = true;
    }
    // если отправитель не игрок, то не будем отправлять статистику
    if(!sender_is_player)
    {
        qDebug() << "The sender is not a player";
        return QString("");
    }
    // добавим расы игроков
    for(int i=0; i<_players_count; ++i)
    {
        site_addr += "r" + QString::number(i+1) + "="
                + QString::number(get_race_id(_players.at(i).race)) + "&";
    }
    // добавим ключ
    site_addr += "key=" + QLatin1String(SERVER_KEY) + "&";
    // добавим имена победителей
    int win_number=1;
    for(int i=0; i<_players_count; ++i)
        if(_players.at(i).fnl_state==5&&win_number<=_type)
        {
//            QByteArray btr = _players.at(i).name.toAscii();
//            QString p_name(btr.toHex());
            QString p_name = _players.at(i).name;
//            qDebug() << "Winner is "+QString(_players.at(i).name[0].toUpper())+_players.at(i).name.mid(1);
            qDebug() << "Winner is "+_players.at(i).name;
            site_addr += "w" + QString::number(win_number) + "="
                    + p_name + "&";
            ++win_number;
        }
    // добавим тип игры
    site_addr += "type=" + QString::number(_type) + "&";
    // добавим имя карты
    site_addr += "map=" + _map_name + "&";
    // добавим продолжительность игры
    site_addr += "gtime=" + QString::number(_duration) + "&";
    // добавим имя отправителя
    site_addr += "name=" + _sender_name + "&";
    // добавим steam id отправителя
    site_addr += "sid=" + _steam_id + "&";
    // добавим названием мода
    site_addr += "mod=" + _mod_name + "&";
    // добавим условие победы
    site_addr += "winby=" + _winby + "&";
    return site_addr;
}

void GameInfo::set_map_name(QString str)
{
    _map_name = str;
}

void GameInfo::set_mod_name(QString name)
{
    _mod_name = name;
}

int GameInfo::get_race_id(QString str)
{
//    int race_id;
//    if(str=="space_marine_race") race_id = 1;
//    if(str=="chaos_marine_race") race_id = 2;
//    if(str=="ork_race") race_id = 3;
//    if(str=="eldar_race") race_id = 4;
//    if(str=="guard_race") race_id = 5;
//    if(str=="necron_race") race_id = 6;
//    if(str=="tau_race") race_id = 7;
//    if(str=="sisters_race") race_id = 8;
//    if(str=="dark_eldar_race") race_id = 9;

    return races.value(str);
}

void GameInfo::add_player(QString name, QString race, int team_id, int state)
{
    Player p;
    // замена пробелов в имени игрока на '_'
    if(name.contains(' ')) name.replace(QRegExp("[^\\w]"),"_");
    p.name = name;
    p.race = race;
    p.team = team_id;
    p.fnl_state = state;
    _players.push_back(p);
}

