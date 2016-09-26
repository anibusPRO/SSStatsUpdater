#ifndef GameInfo_H
#define GameInfo_H

#include <QString>
#include <QVector>
#include <QMap>

#define SERVER_KEY "80bc7622e3ae9980005f936d5f0ac6cc"

struct Player
{
    QString name;
    QString race;
    int team;
    int fnl_state;
};


class GameInfo
{
public:
    GameInfo(int players_count);
    ~GameInfo();
    void add_player(QString name, QString race, int team_id, int state);
    void set_type(int type);
    void set_winby(QString str);
    void set_duration(int time);
    void set_team_number(int num);
    void set_sender_name(QString name);
    void set_steam_id(QString id);
    QString get_url(QString site_addr);
    void set_map_name(QString str);
    void set_mod_name(QString name);

private:
    int get_race_id(QString str);

    QString _map_name;
    QVector<Player> _players;
    int _type;
    QString _winby;
    int _teams_number;
    int _players_count;
    int _duration;
    QString _steam_id;
    QString _sender_name;
    QString _mod_name;

    QMap<QString, int> races;


};

#endif // GameInfo_H
