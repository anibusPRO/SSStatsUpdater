#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

//# define NETWORK_SHOW_SEND_REQUESTS

#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include "apmmeter.h"
#include <QCoreApplication>

#define STEAM_API_KEY "B09655A1E41B5DE93AD3F27087D25884"

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
    void start();

    // для хранения текущего активного профиля
    // имя папки профиля можно узнать так же из файла Local.ini
//    QString _cur_profile;
    // флаг опредляющий выполнение основного цикла работы программы
    bool stop=false;

private:

    void download_map(QString map_name);
    bool download_map_pack();
    QString get_soulstorm_installlocation();
    QString calcMD5(QString fileName);
    QString calcMD5(QByteArray data);
    bool init_player();
    bool send_stats(QString path_to_profile);
    bool send_logfile();
    void processFlags(bool tHP, bool tFog);
    bool decompress(const QString& file, const QString& out, const QString& pwd);
    int updateUpdater();
    QString sender_steamID;
    QString sender_name;
    QString server_addr;
    QString version;
    QString ss_path;
    QCoreApplication *app;
    RequestSender* sender;
    GameInfoReader* reader;
    APMMeter *apm_meter;
    Logger log;
    HANDLE hSharedMemory;
    PGameInfo lpSharedMemory;
    QThread *sender_thread;

signals:
    void start_meter();
    void post(QString url,
              QString name,
              QString content,
              QByteArray data);
    void get(QString url);

private slots:
//    void slotError ( );
    void updateProgress(qint64 bytesSent, qint64 bytesTotal);
    void slotDone (const QUrl&url, const QByteArray&btr);
};

#endif // STATSCOLLECTOR_H
