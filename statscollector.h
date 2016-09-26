#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include <QCoreApplication>
//#include <QObject>

using namespace Network;

class StatsCollector /*: public QObject*/
{
//    Q_OBJECT

public:

    QCoreApplication * app;

    StatsCollector();
    ~StatsCollector();


    bool send_stats(QString path_to_profile);
    bool init_player();

    RequestSender sender;

    GameInfo *info;
    GameInfoReader reader;
    logger log;

//    QString testStats_path;
    QString server_addr;
    QString _cur_profile;
    bool stop=false;
//    QString ss_path;

//public slots:
    void start();

};

#endif // STATSCOLLECTOR_H
