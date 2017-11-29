#ifndef GameInfo_H
#define GameInfo_H

#include <QString>
#include <QVector>
#include <QStringList>
#include <QMap>

struct TSPlayer
{
    QString name;
    QString sid;
    int race;
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

    static QStringList races;
    static QStringList racesUC;

    QString get_winby();
    QString get_stats(QString site_addr="");
    void set_type(int type);
    void set_APMR(double apm);
    void set_APMR(int apm);
    void set_winby(QString str);
    void set_duration(int time);
    void set_team_number(int num);
    void set_sender_name(QString name);
    void set_steam_id(QString id);
    void set_map_name(QString str);
    void set_mod_name(QString name);
    void add_player(QString name, int race, int team_id, int state, QString sid, int apm=0);
    void update_player(int id, int state);

    TSPlayer getPlayer(int id);

private:
    QVector<TSPlayer> _players;
    QString _map_name;

    int _type;
    int _teams_number;
    int _players_count;
    int _duration;
    int apmRi;
    double apmd;
    QString _winby;
    QString _steam_id;
    QString _sender_name;
    QString _mod_name;



};

#endif // GameInfo_H
