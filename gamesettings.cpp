#include "gamesettings.h"
#include <QDebug>

GameSettings::GameSettings()
{
    StartRes      = 0;
    LockTeams     = 0;
    CheatsON      = 1;
    StartLocation = 0;
    GameSpeed     = 2;
    ResShare      = 1;
    ResRate       = 1;
}

GameSettings::~GameSettings()
{

}

bool GameSettings::isStandart(int game_type)
{
    if(StartRes     !=0){
        qDebug() << "StartRes:" << getResolvedResources();
        return false;
    }
    if(game_type!=1&&LockTeams    !=0){
        qDebug() << "LockTeams:" << getResolvedLockTeams();
        return false;
    }
    if(CheatsON     !=1){
        qDebug() << "CheatsON:" << getResolvedCheats();
        return false;
    }
    if(game_type!=1&&StartLocation!=0){
        qDebug() << "StartLocation:" << getResolvedPositions();
        return false;
    }
    if(GameSpeed    !=2){
        qDebug() << "GameSpeed:" << getResolvedGameSpeed();
        return false;
    }
    if(game_type!=1&&ResShare     !=1){
        qDebug() << "ResShare:" << getResolvedResourceSharing();
        return false;
    }
    if(ResRate      !=1){
        qDebug() << "ResRate:" << getResolvedResourceSpeed();
        return false;
    }
    return true;
}


QString GameSettings::getResolvedResources() const
{
    switch (StartRes)
    {
        case 0: return "Standard";
        case 1: return "QuickStart";
        default: return "Unknown";
    }
}

QString GameSettings::getResolvedLockTeams() const
{
    switch (LockTeams)
    {
        case 1: return "Fixed";
        case 0: return "UnFixed";
        default: return "Unknown";
    }
}

QString GameSettings::getResolvedPositions() const
{
    switch (StartLocation)
    {
        case 2: return "Random";
        case 1: return "Fixed";
        default: return "Unknown";
    }
}


QString GameSettings::getResolvedCheats() const
{
    switch (CheatsON)
    {
        case 0: return "No";
        case 1: return "Yes";
        default: return "Unknown";
    }
}


QString GameSettings::getResolvedGameSpeed() const
{
    switch (GameSpeed)
    {
        default: return "Unknown";
    }
}

QString GameSettings::getResolvedResourceSharing() const
{
    switch (ResShare)
    {
        case 1: return "No";
        case 2: return "Yes";
        default: return "Unknown";
    }
}

QString GameSettings::getResolvedResourceSpeed() const
{
    switch (ResRate)
    {
        default: return "Unknown";
    }
}

//LanguageService GameSettings::getLS() const
//{
//    return LS;
//}

QString GameSettings::getResolvedAIDiff() const
{
    switch (AIDiff)
    {
        case 0: return "Hard";
        case 1: return "Insane";
        case 2: return "Easy";
        case 3: return "Hard";
        case 4: return "Insane";
        default: return "Unknown";
    }
}




