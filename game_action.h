#ifndef GAME_ACTION_H
#define GAME_ACTION_H

#include <QString>
#include "replay.h"

class Replay;

class GameAction
{
public:
    GameAction();
    ~GameAction();

    int ActionLen;

    int Kind;
    int KindValue;

    int Tick;

    int Time;

    int PlayerId;

    short PlayerNumber;
    short PlayerNumber2;

    short PlayerActionNumber;

    short ActionType;

    short Value;

    char *AdditionalInfo;
    QByteArray ActionData2;

    QString BuildDescription(Replay replay);

    bool IsForced;

    bool IsUnitOrScructureBuild;

    QString PlayerIdBytes;

    bool Lag;
    bool Chat;
    bool Empty;

    bool getIsUnitOrScructureBuild() const;
};

#endif // GAME_ACTION_H
