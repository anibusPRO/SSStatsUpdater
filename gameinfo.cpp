#include "gameinfo.h"
#include <QDebug>
#include <QRegExp>
#include <QTextCodec>

GameInfo::GameInfo(int players_count)
{
    _players.reserve(players_count);

    races << "space_marine_race"
        <<"chaos_marine_race"
        <<"ork_race"
        <<"eldar_race"
        <<"guard_race"
        <<"necron_race"
        <<"tau_race"
        <<"sisters_race"
        <<"dark_eldar_race";

    _players_count = players_count;
    _map_name = "";
    _type = 0;
    _winby = "";
    _duration = 0;
    _sender_name = "";
    _steam_id = "";
}

TSPlayer GameInfo::getPlayer(int id)
{
   return _players[id];
}

GameInfo::~GameInfo()
{
}

void GameInfo::set_type(int type)
{
    _type = type;
}

void GameInfo::set_APMR(int apm)
{
    apmRi = apm;
}
void GameInfo::set_APMR(double apm)
{
    apmd = apm;
}


void GameInfo::set_winby(QString str)
{
    _winby = str;
}

QString GameInfo::get_winby()
{
    return _winby;
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

QString GameInfo::get_sender_name()
{
    return _sender_name;
}

void GameInfo::set_steam_id(QString id)
{
    _steam_id = id;
}


QString GameInfo::get_steam_id()
{
    qDebug() << "steam id:" << _steam_id;
    return _steam_id;
}

QString GameInfo::get_stats(QString site_addr)
{
    // добавим имена игроков
    for(int i=0; i<_players_count; ++i)
    {

        QByteArray btr = _players.at(i).name.toUtf8();
        QString p_name(btr.toHex());

        // записываем имена игроков
        site_addr += "p" + QString::number(i+1) + "="
                + p_name + "&";
        site_addr += "sid" + QString::number(i+1) + "="
                + _players.at(i).sid + "&";

//        // записываем apm из реплея для каждого игрока
//        site_addr += "apm" + QString::number(i+1) + "="
//                + QString::number(_players.at(i).apm) + "&";

        // записываем реальный apm для игрока который отправляет статистику
//        if(_sender_name==_players.at(i).name)
//            site_addr += "apm"+QString::number(i+1)+"r"+"="+QString::number(apmd)+"&";
    }
    site_addr += "apm="+QString::number(apmd)+"&";
    // добавим расы игроков
    for(int i=0; i<_players_count; ++i)
    {
        site_addr += "r" + QString::number(i+1) + "="
                + QString::number( races.indexOf(_players.at(i).race) + 1) + "&";
    }
    // добавим имена победителей
    int win_counter=1;
    for(int i=0; i<_players_count; ++i)
        if(_players.at(i).fnl_state==5&&win_counter<=_type)
        {
            QByteArray btr = _players.at(i).name.toUtf8();
            QString p_name(btr.toHex());

            site_addr += "w" + QString::number(win_counter) + "="
                    + p_name + "&";
            ++win_counter;
        }

    // добавим тип игры
    site_addr += "type=" + QString::number(_type) + "&";
    // добавим имя карты
    site_addr += "map=" + _map_name + "&";
    // добавим продолжительность игры
    site_addr += "gtime=" + QString::number(_duration) + "&";
    // добавим steam id отправителя
    site_addr += "sid=" + _steam_id + "&";
    // добавим названием мода
    site_addr += "mod=" + _mod_name + "&";
    // добавим условие победы
    site_addr += "winby=" + _winby + "&";

    qDebug() << site_addr;
    return site_addr;
}

void GameInfo::set_map_name(QString str)
{
    qDebug() << "Map name:" << str;
    _map_name = str;
}

void GameInfo::set_mod_name(QString name)
{
    _mod_name = name;
}

void GameInfo::update_player(int id, int state)
{
    _players[id].fnl_state = state;
}

void GameInfo::add_player(QString name, QString race, int team_id, int state, QString sid, int apm)
{
    qDebug() << "Adding player:" << name << race << team_id << state << sid;
    TSPlayer p;
    p.name = name;
    p.sid = sid;
    p.race = race;
    p.team = team_id;
    p.apm = apm;
    p.fnl_state = state;
    _players.push_back(p);
}

