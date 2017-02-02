#ifndef GameInfoReader_H
#define GameInfoReader_H

#include <QDebug>
#include <QVariantMap>
#include <QByteArray>
#include <QString>
#include "repreader.h"
#include <QTime>
#include "gameinfo.h"
#include <QStringList>

#ifdef HAVE_QT5
#   include <QJsonDocument>
#else
#   include "json.h"
#endif

#define STEAM_API_KEY "B09655A1E41B5DE93AD3F27087D25884"

#define MINIMUM_DURATION 30


class GameInfoReader
{
public:
    GameInfoReader();
    ~GameInfoReader();

    QString get_url(QString profile, QString path_to_playback);
    int get_game_info(QString profile, QString path_to_playback);
    QStringList get_sender_name(bool init=false);
    QString read_warnings_log(QString str, int offset=0);
    QString get_cur_profile_dir();
    QByteArray get_playback_file();

    void setAverageAPM(int apm);
    int readySend();
    bool timeCompare(QTime t1, QTime t2);
    int timeDifference(QTime t1, QTime t2);
    bool tempRecExist();
    QTime last_playback;
    QTime last_startgame;
    QTime last_stopgame;
    bool stopgame_valid;
    int average_apm;

    void set_ss_path(const QString &value);
    QString get_steam_id();
    void set_server_addr(QString addr);

private:
    QString server_addr;
    GameInfo *_game_info;
    QString cur_profile_name;
    QString ss_path;
    QStringList errors_list;
    QByteArray _playback;
    QStringList steam_id64;
    int error_code;

};

#endif // GameInfoReader_H
