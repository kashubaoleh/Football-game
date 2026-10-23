//Match.h
#pragma once
#include "Team.h"
#include "MatchLog.h"

struct Match {
    Team homeTeam;
    Team awayTeam;
    bool isPlayed = false;
    MatchLog resultLog;
};
