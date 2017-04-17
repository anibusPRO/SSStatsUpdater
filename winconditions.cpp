#include "winconditions.h"
#include <QDebug>

WinConditions::WinConditions()
{
    hasAnnihilate       = true;
    hasDestroyHQ        = false;
    hasEconomicVictory  = false;
    hasSuddenDeath      = false;
    hasAssassinate      = false;
}

WinConditions::~WinConditions()
{

}

bool WinConditions::isStandart(int game_type)
{
    if(!hasAnnihilate){
        qDebug() << "Win conditions do not contain \"Annihilate\"";
        return false;
    }
    if(hasDestroyHQ){
        qDebug() << "Win conditions contain \"Destroy HQ\"";
        return false;
    }
    if(hasEconomicVictory){
        qDebug() << "Win conditions contain \"Economic Victory\"";
        return false;
    }
    if(hasSuddenDeath){
        qDebug() << "Win conditions contain \"Sudden Death\"";
        return false;
    }
    if(hasAssassinate){
        qDebug() << "Win conditions contain \"Assassinate\"";
        return false;
    }
    return true;
}

