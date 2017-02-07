#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

//# define NETWORK_SHOW_SEND_REQUESTS

#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include "apmmeter.h"
#include <QCoreApplication>

using namespace Network;

class StatsCollector : public QObject
{
    Q_OBJECT
public:
    StatsCollector();
    ~StatsCollector();
    void start();

    // для хранения текущего активного профиля
    // имя папки профиля можно узнать так же из файла Local.ini
//    QString _cur_profile;
    // флаг опредляющий выполнение основного цикла работы программы
    bool stop=false;

private:
    QCoreApplication *app;
    APMMeter *apm_meter;
    // получает путь до игры из реестра windows
    QString get_soulstorm_installlocation();
    // отправляет статистику взятую из файлв testStats.lua в папке path_to_profile
    bool send_stats(QString path_to_profile, QString path_to_playback);
    bool send_logfile();
    // инициализирует игрока на сервере путем отправки steam_id игрока
    bool init_player();
    int updateUpdater();
//    GameInfo *info;
    GameInfoReader reader;
    logger log;
    RequestSender sender;
    // для хранения адреса сервера
    QString server_addr;
    QString version;
signals:
    void start_meter();
};

#endif // STATSCOLLECTOR_H
