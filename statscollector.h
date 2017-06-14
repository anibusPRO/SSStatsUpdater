#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

//# define NETWORK_SHOW_SEND_REQUESTS

#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include "apmmeter.h"
#include "monitor.h"
#include <QCoreApplication>
#include <QTimer>
#include "defines.h"


typedef struct{
    int AverageAPM;
    int CurrentAPM;
    int MaxAPM;
    int downloadProgress;
    char players[8][100];
    char mapName[50];
} TGameInfo;

typedef TGameInfo *PGameInfo;

class StatsCollector : public QObject
{
    Q_OBJECT
public:
    StatsCollector(QObject* pobj=0);
    ~StatsCollector();

    bool start();
    // флаг опредляющий выполнение основного цикла работы программы
    bool stop=false;
    QTimer stats_timer;

private:

    void download_map(QString map_name, bool enableDXHook);
    QString get_soulstorm_installlocation();
    QString calcMD5(QString fileName);
    QString calcMD5(QByteArray data);
    bool init_player();
    bool send_stats(QString path_to_profile);
    bool send_logfile();
    void processFlags();

    QString sender_steamID;
    QString sender_name;
    QString server_addr;
    bool enableDXHook;
    bool enableStats;
    bool curFog;
    bool curHP;
    QString version;
    QString ss_path;
    QCoreApplication *app;
    RequestSender* sender;
    GameInfoReader* reader;
    Logger log;
    HANDLE hSharedMemory;
    PGameInfo lpSharedMemory;
    QThread *sender_thread;
    QThread* apm_thread;
    QThread* monitor_thread;
    QDateTime cur_time;
    Monitor monitor;
    APMMeter apm_meter;

signals:
    void start_apm_meter();
    void start_monitor();
    void POST_REQUEST(QString url,
              QString name,
              QString content,
              QByteArray data);
//    void GET_REQUEST(QString url);
    void GET_REQUEST(QString url, QString fileName="");

private slots:
    void check_game();
};

#endif // STATSCOLLECTOR_H
