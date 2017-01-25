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
    if(!hasAnnihilate)
        return false;
    if(hasDestroyHQ)
        return false;
    if(hasEconomicVictory)
        return false;
    if(hasSuddenDeath)
        return false;
    if(hasAssassinate)
        return false;
    return true;
}

void WinConditions::debug()
{
    qDebug() << "hasDestroyHQ:      " << hasDestroyHQ      ;
    qDebug() << "hasEconomicVictory:" << hasEconomicVictory;
    qDebug() << "hasSuddenDeath:    " << hasSuddenDeath    ;
    qDebug() << "hasAnnihilate:     " << hasAnnihilate     ;
    qDebug() << "hasAssassinate:    " << hasAssassinate    ;
}

