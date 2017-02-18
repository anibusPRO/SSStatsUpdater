#include "replay.h"
#include <QStringList>
#include <QDebug>

Replay::Replay(QString fullFileName)
{
//    Players = new QList<Player>();
    settings = new GameSettings();
    conditions = new WinConditions();

//    ChatMSG = new QList<ChatMessage>();

//    BeginPlayerDiffirences = new QList<long>();
//    BeginFOLDGPLYz = new QList<long>();
//    BeginPlayersChunkDataSizes = new QMap<long, int>();
//    Actions = new QList<GameAction>();
    ReadedFully = false;
    FullFileName = fullFileName;
}

Replay::~Replay()
{
//    delete ChatMSG;
    delete conditions;
    delete settings;
//    delete Players;
//    delete BeginPlayerDiffirences;
//    delete BeginFOLDGPLYz;
//    delete BeginPlayersChunkDataSizes;
//    delete Actions;
}
QString Replay::getMapLocale() const
{
    return MapLocale;
}

void Replay::setMapLocale(const QString &value)
{
    MapLocale = value;
}

QString Replay::getHash() const
{
    return Hash;
}

void Replay::setHash(const QString &value)
{
    Hash = value;
}

bool Replay::getReadedFully() const
{
    return ReadedFully;
}

void Replay::setReadedFully(bool value)
{
    ReadedFully = value;
}

long Replay::getBeginVersion() const
{
    return BeginVersion;
}

void Replay::setBeginVersion(long value)
{
    BeginVersion = value;
}

int Replay::getBeginNAME() const
{
    return BeginNAME;
}

void Replay::setBeginNAME(int value)
{
    BeginNAME = value;
}

long Replay::getBeginFOLDINFO() const
{
    return BeginFOLDINFO;
}

void Replay::setBeginFOLDINFO(long value)
{
    BeginFOLDINFO = value;
}

int Replay::getLengthFOLDINFO() const
{
    return LengthFOLDINFO;
}

void Replay::setLengthFOLDINFO(int value)
{
    LengthFOLDINFO = value;
}

long Replay::getBeginDATABASE() const
{
    return BeginDATABASE;
}

void Replay::setBeginDATABASE(long value)
{
    BeginDATABASE = value;
}

int Replay::getLengthDATABASE() const
{
    return LengthDATABASE;
}

void Replay::setLengthDATABASE(int value)
{
    LengthDATABASE = value;
}

long Replay::getBeginFOLDWMAN() const
{
    return BeginFOLDWMAN;
}

void Replay::setBeginFOLDWMAN(long value)
{
    BeginFOLDWMAN = value;
}

int Replay::getLengthFOLDWMAN() const
{
    return LengthFOLDWMAN;
}

void Replay::setLengthFOLDWMAN(int value)
{
    LengthFOLDWMAN = value;
}

long Replay::getBeginDataBaseChunkSize() const
{
    return BeginDataBaseChunkSize;
}

void Replay::setBeginDataBaseChunkSize(long value)
{
    BeginDataBaseChunkSize = value;
}

int Replay::getDataBaseChunkSize() const
{
    return DataBaseChunkSize;
}

void Replay::setDataBaseChunkSize(int value)
{
    DataBaseChunkSize = value;
}

QString Replay::getName() const
{
    return Name;
}

void Replay::setName(const QString &value)
{
    Name = value;
}

int Replay::getPlayerCount() const
{
    return PlayerCount;
}

void Replay::setPlayerCount(int value)
{
    PlayerCount = value;
}

int Replay::getMapSize() const
{
    return MapSize;
}

void Replay::setMapSize(int value)
{
    MapSize = value;
}

QString Replay::getMap() const
{
    return Map;
}

QString Replay::getShortMapName()
{
    QString name="";
    QStringList lst = Map.right(Map.size()-2).split('_');
    foreach (QString word, lst)
        name += word.left(1).toUpper();
    return name;
}

void Replay::setMap(const QString &value)
{
    Map = value;
}

int Replay::getVersion() const
{
    return Version;
}

void Replay::setVersion(int value)
{
    Version = value;
}

QString Replay::getMOD() const
{
    return MOD;
}

void Replay::setMOD(const QString &value)
{
    MOD = value;
}

long Replay::getActionDBSize() const
{
    return ActionDBSize;
}

void Replay::setActionDBSize(long value)
{
    ActionDBSize = value;
}

int Replay::getTotalTicks() const
{
    return TotalTicks;
}

void Replay::setTotalTicks(int value)
{
    TotalTicks = value;
}

int Replay::getSlots() const
{
    return Slots;
}

void Replay::setSlots(int value)
{
    Slots = value;
}

long Replay::getPlayerStart() const
{
    return PlayerStart;
}

void Replay::setPlayerStart(long value)
{
    PlayerStart = value;
}

long Replay::getActionStart() const
{
    return ActionStart;
}

void Replay::setActionStart(long value)
{
    ActionStart = value;
}

int Replay::getActionCount() const
{
    return ActionCount;
}

void Replay::setActionCount(int value)
{
    ActionCount = value;
}

QString Replay::getFullFileName() const
{
    return FullFileName;
}

void Replay::setFullFileName(const QString &value)
{
    FullFileName = value;
}

//bool Replay::getHasException() const
//{
//    return Exception != null;
//}


QString Replay::getConditionsQString() const
{
//    var builder = new QStringBuilder();
    QString builder;

    if (conditions->Annihilate)
        builder += "Annihilate ";

    if (conditions->Assassinate)
        builder += "Assassinate ";

    if (conditions->ControlArea)
        builder += "ControlArea ";

    if (conditions->DestroyHQ)
        builder += "DestroyHQ ";

    if (conditions->EconomicVictory)
        builder += "EconomicVictory ";

    if (conditions->GameTimer)
        builder += "GameTimer ";

    if (conditions->SuddenDeath)
        builder += "SuddenDeath ";

    if (conditions->TakeAndHold)
        builder += "TakeAndHold ";

    return builder;
}


QString Replay::getBuildName() const
{
    //    if (this.Exception == null)
    //        return (this.Name ?? "UNKNOWN")+"                                                                         "+Id.ToQString();
//    else
//        return "EXCEPTION : " + this.FullFileName ?? "";
}


bool Replay::getIs_1_2_Version() const
{
    return Version == 9;
}


bool Replay::getIsSteam() const
{
    return Version == 10;
}

int Replay::getTeamsCount() const
{
    if (Players.isEmpty())
        return 0;

    int count = 0;

//    foreach(Player item, &Players)
    for(int i=0; i<Players.size(); ++i)
        count = qMax(count, Players.at(i)->Team);

    return count;
}


int Replay::GetPlayerMidApm(int id)
{
//    Player *pl = Players.at(id);


    if (/*pl == 0 || pl->isEmpty() || pl->isSpectator() || */id >= PlayerCount || Actions.isEmpty() || Actions.size() == 0)
    {
//        qDebug() << Players[id]->getResolvedTypeStr();
        qDebug() << "Bad id or Actions is empty";
        return 0;
    }

    GameAction *last = Actions.last();
    auto ticksCount = last->Tick;

    int acts_count = 0;
    for(int i=0; i<Actions.size(); ++i)
        if(Actions[i]->PlayerNumber == id)
            ++acts_count;

    int apm = (int)(((double)acts_count/ ticksCount) * 8 * 60);

    return apm;
}

bool Replay::IsAnyAverageAPMAbove(int apm)
{
    if (PlayerCount == 0 || TotalTicks == 0)
        return false;
    return (((ActionDBSize * 8 * 60 / 80) / PlayerCount) / TotalTicks) >= apm;
}
