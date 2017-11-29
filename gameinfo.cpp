#include "gameinfo.h"
#include <QDebug>
#include <QRegExp>
#include <QTextCodec>
#include <QUrl>

QStringList GameInfo::races = QStringList() << "random" << "space_marine_race"
    <<"chaos_marine_race" <<"ork_race" <<"eldar_race" <<"guard_race"
    <<"necron_race" <<"tau_race" <<"sisters_race" <<"dark_eldar_race";
QStringList GameInfo::racesUC = QStringList() << "Random" << "Space Marines"
    <<"Chaos" <<"Orks" <<"Eldar" <<"Imperial Guard" <<"Necrons"
    <<"Tau Empire" <<"Sisters of Battle" <<"Dark Eldar";


GameInfo::GameInfo(int players_count)
{
    _players.reserve(players_count);
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

void GameInfo::set_steam_id(QString id)
{
    _steam_id = id;
}

QString GameInfo::get_stats(QString site_addr)
{
    int win_counter=0;
    // имена, сиды, расы игроков и победитлей
    for(int i=0; i<_players_count; ++i)
    {
        site_addr += QString("p%1=%2&").arg(i+1).arg(QString(QUrl::toPercentEncoding(_players.at(i).name)));
        if(!_players.at(i).sid.isEmpty())
            site_addr += QString("sid%1=%2&").arg(i+1).arg(_players.at(i).sid);
        else if(_players.at(i).name==_sender_name)
            site_addr += QString("sid%1=%2&").arg(i+1).arg(_steam_id);
        site_addr += QString("r%1=%2&").arg(i+1).arg(_players.at(i).race);
        if(_players.at(i).fnl_state==5&&win_counter<_type)
            site_addr += QString("w%1=%2&").arg(++win_counter).arg(i+1);
    }

    site_addr += QString("apm=%1&").arg(apmd); // апм отправителя
    site_addr += QString("type=%1&").arg(_type); // тип игры
    site_addr += QString("map=%1&").arg(QString(QUrl::toPercentEncoding(_map_name))); // имя карты
    site_addr += QString("gtime=%1&").arg(_duration); // продолжительность игры
    site_addr += QString("sid=%1&").arg(_steam_id); // steam id отправителя
    site_addr += QString("mod=%1&").arg(QString(QUrl::toPercentEncoding(_mod_name))); // названием мода
    site_addr += QString("winby=%1&").arg(QString(QUrl::toPercentEncoding(_winby))); // условие победы

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

void GameInfo::add_player(QString name, int race, int team_id, int state, QString sid, int apm)
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

