#include "gamesettings.h"
#include <QDebug>

GameSettings::GameSettings()
{

}

GameSettings::~GameSettings()
{

}

bool GameSettings::isStandart()
{
    if(StartRes     !=0)
        return false;
    if(LockTeams    !=0)
        return false;
    if(CheatsON     !=1)
        return false;
    if(StartLocation!=0)
        return false;
    if(GameSpeed    !=2)
        return false;
    if(ResShare     !=1)
        return false;
    if(ResRate      !=1)
        return false;
    return true;
}


QString GameSettings::getResolvedResources() const
{
    switch (StartRes)
    {
    case 0: return LS.Std;
    case 1: return LS.QuickStart;
    default: return LS.Unknown;
    }
}

QString GameSettings::getResolvedLockTeams() const
{
    switch (LockTeams)
    {
    case 1: return LS.Fixed;
    case 0: return LS.UnFixed;
    default: return LS.Unknown;
    }
}

QString GameSettings::getResolvedPositions() const
{
    switch (StartLocation)
    {
    case 2: return LS.Random;
    case 1: return LS.Fixed;
    default: return LS.Unknown;
    }
}


QString GameSettings::getResolvedCheats() const
{
    switch (CheatsON)
    {
    case 0: return LS.No;
    case 1: return LS.Yes;
    default: return LS.Unknown;
    }
}


QString GameSettings::getResolvedGameSpeed() const
{
    switch (GameSpeed)
    {
    default: return LS.Std;
    }
}

QString GameSettings::getResolvedResourceSharing() const
{
    switch (ResShare)
    {
    case 1: return LS.No;
    case 2: return LS.Yes;
    default: return LS.Unknown;
    }
}

QString GameSettings::getResolvedResourceSpeed() const
{
    switch (ResRate)
    {
    default: return LS.Std;
    }
}

LanguageService GameSettings::getLS() const
{
    return LS;
}

QString GameSettings::getResolvedAIDiff() const
{
    switch (AIDiff)
    {
    case 0: return LS.Hard;
    case 1: return LS.Insane;
        case 2: return LS.Easy;
        case 3: return LS.Hard;
        case 4: return LS.Insane;
        default: return LS.Unknown;
    }
}




