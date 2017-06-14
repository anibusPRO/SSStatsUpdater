#ifndef GameInfoReader_H
#define GameInfoReader_H

#include <QDebug>
#include <QVariantMap>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTime>

#include "repreader.h"
#include "gameinfo.h"


#ifdef HAVE_QT5
#   include <QJsonDocument>
#else
#   include "json.h"
#endif

#define MINIMUM_DURATION 30


class GameInfoReader
{
public:
    GameInfoReader();
    ~GameInfoReader();

    void init(QString path, QString SID, QString name);
    void reset();
    QString read_warnings_log(QString str, int offset=0, int count=1);
    QString get_cur_profile_dir();
    QString find_profile();
    QString get_playback_name() const;
    QString get_last_invalid_map() const;
    QString get_game_stats() const;
    QStringList get_players();
    QByteArray get_playback_file();
    bool is_map_valid();
    void get_error_debug(int e_code);
    int read_game_info(QMap<QString, QString> *sids, long totalActions);
    int readySend();

    QString cur_profile_folder;

private:
    int search_info(long totalActions);
    QString game_stats;
    QString ss_path;
    QString playback_name;
    QString sender_steamID;
    QString sender_name;
    QString last_invalid_map;
    QStringList errors_list;
    QByteArray _playback;

    bool is_playback;
    bool stopgame_valid;
    // для того чтобы ошибка из профиля выводилась только один раз
    bool profile_error;
    int last_playback;
    int last_startgame;
    int last_stopgame;
    int last_ending_mission;
    int playback_error;
    int average_apm;
    int error_code;
    long TotalActions;

};

#endif // GameInfoReader_H
