#ifndef GameInfo_H
#define GameInfo_H

#include <QString>
#include <QVector>
#include <QMap>

#define SERVER_KEY "80bc7622e3ae9980005f936d5f0ac6cd"

struct TSPlayer
{
    QString name;
    QString race;
    int team;
    int apm;
    int apmr;
    int fnl_state;
};


class GameInfo
{
public:
    GameInfo(int players_count);
    ~GameInfo();
    void add_player(QString name, QString race, int team_id, int state, int apm=0);
    void set_type(int type);
    void setAPMR(double apm);
    void setAPMR(int apm);
    void set_winby(QString str);
    QString get_winby();
    void set_duration(int time);
    void set_team_number(int num);
    void set_sender_name(QString name);
    QString get_sender_name();
    void set_steam_id(QString id);
    QString get_steam_id();
    QString get_url(QString site_addr);
    void set_map_name(QString str);
    void set_mod_name(QString name);
    void update_player(int id, int state);

    static QString toUtf16Hex(QString str);

    TSPlayer getPlayer(int id);

private:

    QVector<TSPlayer> _players;    QString _map_name;
//    QMap<QString, int> races;
    QVector<QString> races;

    int _type;
    int _teams_number;
    int _players_count;
    int _duration;
    int apmRi;
    int apmRd;
    QString _winby;
    QString _steam_id;
    QString _sender_name;
    QString _mod_name;



};

#endif // GameInfo_H
