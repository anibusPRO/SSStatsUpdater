#include "game_action.h"

GameAction::GameAction()
{

}

GameAction::~GameAction()
{

}

QString GameAction::BuildDescription(Replay replay)
{
//    if (IsForced)
//        return "-- FORCED ACTION --";

//    return "[" + Time + "] " + PlayerActionNumber + " | " + replay.Players.at(PlayerNumber)->Name + " | " + ActionsService.GetActionDescription(Kind, KindValue) + " " + ActionType + " " + Value;
}

bool GameAction::getIsUnitOrScructureBuild() const
{
    return Kind == 117 || Kind == 3 || (Kind == 4 && KindValue == 3);
}

