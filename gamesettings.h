#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QString>
#include "languageservice.h"

class GameSettings
{
public:
    GameSettings();
    ~GameSettings();

    int AIDiff;
    int StartRes;
    int LockTeams;
    int CheatsON;
    int StartLocation;
    int GameSpeed;
    int ResShare;
    int ResRate;

    QString ResolvedAIDiff;

    QString ResolvedResources;

    QString ResolvedLockTeams;

    QString ResolvedPositions;

    QString ResolvedCheats;

    QString ResolvedGameSpeed;

    QString ResolvedResourceSharing;

    QString ResolvedResourceSpeed;

//    LanguageService getLS() const;

    QString getResolvedAIDiff() const;

    QString getResolvedResourceSpeed() const;

    QString getResolvedResourceSharing() const;

    QString getResolvedGameSpeed() const;

    QString getResolvedCheats() const;

    QString getResolvedResources() const;

    QString getResolvedLockTeams() const;

    QString getResolvedPositions() const;

    bool isStandart(int game_type);

//private:
//    LanguageService LS;

};

#endif // GAMESETTINGS_H
