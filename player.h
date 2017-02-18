#ifndef PLAYER_H
#define PLAYER_H

#include <QString>

class Player
{
public:
    Player();
    ~Player();

    enum RaceEnum
    {
        Unknown = 0,
        SpaceMarines,
        Orks,
        Eldar,
        ChaosSpaceMarines,
        DarkEldar,
        Necrons,
        SistersOfBattle,
        Tau,
        ImperialGuard
    };
    enum PlayerTypeEnum
    {
        EmptySlot = 0,
        Host,
        Computer,
        Spectator,
        OtherPlayer
    };


    short ActionCount;

    int LastActionTick;

    int Type;

    QString Name;

    int Team;

    QString Race;


    RaceEnum ResolvedRace;

    QString Bytes;
    char Temp;
    int getResolvedType() const;
    QString getResolvedTypeStr() const;
    int getResolvedRace() const;
    QString getShortRaceName() const;
    QString getVeryShortRaceName() const;
    bool isEmpty() const;
    bool isSpectator() const;
};

#endif // PLAYER_H
