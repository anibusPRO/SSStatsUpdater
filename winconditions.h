#ifndef WINCONDITIONS_H
#define WINCONDITIONS_H


class WinConditions
{
public:
    WinConditions();
    ~WinConditions();

    enum ConditionValues
    {
        Annihilate = 1003066394,
//                     1003066394
        SuddenDeath = -1826760460,
        Assassinate = 200405640,
        EconomicVictory = -242444938,
        ControlArea = 735076042,
        DestroyHQ = 1509920563,
        TakeAndHold = 1959084950,
        GameTimer = 69421273
    };

    //[DWORD] // Win conditon
    //767227721 Annihilate
    //-1826760460 SuddenDeath
    //200405640 Assassinate
    //-242444938 EconomicVictory
    //735076042 ControlArea
    //1509920563 DestroyHQ
    //1959084950 TakeandHolde
    //69421273 GameTime

    bool hasTakeAndHold;

    bool hasDestroyHQ;

    bool hasControlArea;

    bool hasEconomicVictory;

    bool hasSuddenDeath;

    bool hasAnnihilate;

    bool hasAssassinate;

    bool hasGameTimer;

    bool isStandart(int game_type);
    void debug();
};

#endif // WINCONDITIONS_H
