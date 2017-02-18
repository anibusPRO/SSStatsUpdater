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
    bool init_player();
    // искомая строка, отступ от первого слова искомой строки, количество слов в результате
    QString read_warnings_log(QString str, int offset=0, int count=1);
    QString get_cur_profile_dir();
    QByteArray get_playback_file();
    QString get_playback_name();
    void setAverageAPM(int apm);
    void setTotalActions(long n);
    int readySend();
    int last_playback;
    int last_startgame;
    int last_stopgame;
    int playback_error;
    bool is_playback;
    bool stopgame_valid;
    int average_apm;
    long TotalActions;
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
    QString playback_name;
    QMap<QString, QString> names_steamids;
    QString sender_steam_id;
    int error_code;

};

#endif // GameInfoReader_H
