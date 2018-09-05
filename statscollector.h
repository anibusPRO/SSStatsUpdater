#ifndef STATSCOLLECTOR_H
#define STATSCOLLECTOR_H

//# define NETWORK_SHOW_SEND_REQUESTS
#include "requestsender.h"
#include "gameinforeader.h"
#include "gameinfo.h"
#include "logger.h"
#include "apmmeter.h"
#include <QCoreApplication>
#include <QTimer>
#include <QThread>
#include "defines.h"
#include "qtxglobalshortcut/qxtglobalshortcut.h"

typedef struct{
    char name[100];
    int race;
    int gamesCount;
    int winsCount;
    int winRate;
    int mmr;
    int mmr1v1;
    int apm;
} TPlayer;

typedef struct{
    bool enableDXHook;
    int version;
    char players[8][100];
    int playersNumber;
    TPlayer lobbyPlayers[50];
    char mapName[50];
    int AverageAPM;
    int CurrentAPM;
    int MaxAPM;
    int downloadProgress;
    bool fontsInited;
    bool showMenu;
    bool showRaces;
    bool showAPM;
    DWORD statsThrId;
    PDWORD sidsAddr[10];
    bool sidsAddrLock;
} TGameInfo;

typedef TGameInfo *PGameInfo;

class StatsCollector : public QObject
{
    Q_OBJECT
public:
    explicit StatsCollector(QObject *parent = 0);
    ~StatsCollector();

    bool start();
    // флаг опредляющий выполнение основного цикла работы программы
    bool stop=false;
    QTimer stats_timer;

private:
    void check_name();
    void download_map(QString map_name);
    void register_player(QString name, QString sid, bool init=false);
    QString GetRandomString() const;
    QString get_soulstorm_installlocation();
    QString calcMD5(QString fileName);
    QString calcMD5(QByteArray data);
    bool init_player();
    bool send_stats(QString path_to_profile);
    bool send_logfiles();
    bool removeDir(const QString & dirName);
    void processFlags(bool force=false);
    int GetSteamPlayersInfo(bool get_stats=true);

    QString sender_steamID;
    QString sender_name;
    QString server_addr;
    bool closeWithGame;
    bool enableDXHook;
    bool enableStats;
    bool showMenu;
    bool showRaces;
    bool showAPM;
    bool curFog;
    bool curHP;
    bool gameGoing;
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
    APMMeter apm_meter;
    BYTE steamHeader[18] =  { 0x18, 0x0, 0x0, 0x0, 0x2F, 0x0, 0x73, 0x0, 0x74, 0x0, 0x65, 0x0, 0x61, 0x0, 0x6D, 0x0, 0x2F, 0x0 };
    QStringList PlayersInfo;
    QMap<QString, QString> AllPlayersInfo;
    QMap<DWORD, DWORD> moduleInfo;
    bool useOldSIDSearch;

signals:
    void start_apm_meter();
    void start_monitor();
    void POST_REQUEST(QString url,
              QString name,
              QString content,
              QByteArray data,
              QString mapping);
    void GET_REQUEST(QString url, QString fileName="");

private slots:
    void check_game_steam();
    void check_game_tunngle();
    void updateProgress(qint64 bytesSent, qint64 bytesTotal);
    void toggleMenuVisibility();
    void toggleRacesVisibility();
    void toggleAPMVisibility();

public slots:
    void exitHandler();
};

#endif // STATSCOLLECTOR_H
