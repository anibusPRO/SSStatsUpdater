#ifndef Replay_H
#define Replay_H
#include <QString>
#include <QMap>
#include "game_action.h"
#include "player.h"
#include "gamesettings.h"
#include "winconditions.h"

class GameAction;

class Replay
{
public:
    Replay(QString fullFileName);
    ~Replay();

    QString Hash;

    bool ReadedFully;

    QList<GameAction *> Actions;

    long BeginVersion;

    int BeginNAME;
    long BeginFOLDINFO;
    int LengthFOLDINFO;

    long BeginDATABASE;
    int LengthDATABASE;

    long BeginFOLDWMAN;
    int LengthFOLDWMAN;

    int DataBaseChunkSize;
    long BeginDataBaseChunkSize;

    QList<long> BeginFOLDGPLYz;
    QList<long> BeginPlayerDiffirences;
    QMap<long, int> BeginPlayersChunkDataSizes;

    QString Name;
    int Duration;
    int PlayerCount;
    int MapSize;
    QString MapLocale;
    QString Map;
    int Version;
    QString MOD;
    long ActionDBSize;
    int TotalTicks;

    int Slots;

    long ActionStart;
    long PlayerStart;

    WinConditions *conditions;
    QList<Player *> Players;
    GameSettings *settings;

    int TeamsCount;

    int ActionCount;
    QString FullFileName;

    bool IsSteam;

    bool Is_1_2_Version;

    QString BuildName;
    QString ConditionsQString;

//    bool HasException;

    int GetPlayerMidApm(int id);

    bool IsAnyAverageAPMAbove(int apm);

    bool getIsSteam() const;
    bool getIs_1_2_Version() const;
    QString getBuildName() const;
    QString getConditionsQString() const;
    int getTeamsCount() const;
    QString getFullFileName() const;
    void setFullFileName(const QString &value);
    int getActionCount() const;
    void setActionCount(int value);
    long getActionStart() const;
    void setActionStart(long value);
    long getPlayerStart() const;
    void setPlayerStart(long value);
    int getSlots() const;
    void setSlots(int value);
    int getTotalTicks() const;
    void setTotalTicks(int value);
    long getActionDBSize() const;
    void setActionDBSize(long value);
    QString getMOD() const;
    void setMOD(const QString &value);
    int getVersion() const;
    void setVersion(int value);

    QString getShortMapName() const;
    QString getMap() const;
    void setMap(const QString &value);

    int getMapSize() const;
    void setMapSize(int value);

    int getPlayerCount() const;
    void setPlayerCount(int value);

    QString getName() const;
    void setName(const QString &value);

    int getDataBaseChunkSize() const;
    void setDataBaseChunkSize(int value);

    long getBeginDataBaseChunkSize() const;
    void setBeginDataBaseChunkSize(long value);

    int getLengthFOLDWMAN() const;
    void setLengthFOLDWMAN(int value);

    long getBeginFOLDWMAN() const;
    void setBeginFOLDWMAN(long value);

    int getLengthDATABASE() const;
    void setLengthDATABASE(int value);

    long getBeginDATABASE() const;
    void setBeginDATABASE(long value);

    int getLengthFOLDINFO() const;
    void setLengthFOLDINFO(int value);

    long getBeginFOLDINFO() const;
    void setBeginFOLDINFO(long value);

    int getBeginNAME() const;
    void setBeginNAME(int value);

    long getBeginVersion() const;
    void setBeginVersion(long value);

    bool getReadedFully() const;
    void setReadedFully(bool value);

    QString getHash() const;
    void setHash(const QString &value);

    QString getMapLocale() const;
    void setMapLocale(const QString &value);
};

#endif // Replay_H
