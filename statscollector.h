#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include <QCoreApplication>

using namespace Network;

class StatsCollector
{
public:
    StatsCollector();
    ~StatsCollector();
    void start();

    // для хранения адреса сервера
    QString server_addr;
    // для хранения текущего активного профиля
    // имя папки профиля можно узнать так же из файла Local.ini
    QString _cur_profile;
    // флаг опредляющий выполнение основного цикла работы программы
    bool stop=false;

private:
    QCoreApplication * app;

    // получает путь до игры из реестра windows
    QString get_soulstorm_installlocation();
    // отправляет статистику взятую из файлв testStats.lua в папке path_to_profile
    bool send_stats(QString path_to_profile);
    // инициализирует игрока на сервере путем отправки steam_id игрока
    bool init_player();

    GameInfo *info;
    GameInfoReader reader;
    logger log;
    RequestSender sender;

};

#endif // STATSCOLLECTOR_H
