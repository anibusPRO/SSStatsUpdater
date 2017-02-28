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

    QString get_game_info(QString profile);
    QString read_warnings_log(QString str, int offset=0, int count=1);
    QString get_cur_profile_dir();
    QString get_steam_id();
    QString get_playback_name();
    QByteArray get_playback_file();
    void setTotalActions(long n);
    void set_ss_path(const QString &value);
    void set_accounts(QMap<QString, QString> map);
    int readySend();

private:
    int search_info(QString profile);
    GameInfo *_game_info;
    QString sender_steam_id;
    QString cur_profile_name;
    QString ss_path;
    QString playback_name;
    QStringList errors_list;
    QByteArray _playback;
    QMap<QString, QString> accounts;

    bool is_playback;
    bool stopgame_valid;
    int last_playback;
    int last_startgame;
    int last_stopgame;
    int playback_error;
    int average_apm;
    int error_code;
    long TotalActions;

};

#endif // GameInfoReader_H
