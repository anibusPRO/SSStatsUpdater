#include "winconditions.h"
#include <QDebug>

WinConditions::WinConditions()
{

}

WinConditions::~WinConditions()
{

}

bool WinConditions::isStandart()
{
    if(hasDestroyHQ)
        return false;
    if(hasEconomicVictory)
        return false;
    if(hasSuddenDeath)
        return false;
    if(!hasAnnihilate)
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

