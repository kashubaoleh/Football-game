// MatchSimulator.h
#pragma once
#include "../core/Team.h"
#include "../core/MatchLog.h"
#include "../core/Config.h"
#include "Random.h"

class MatchSimulator {
private:
    void simulateTimeBlock(
        int startMin, int endMin, int extraMinutes,
        int baseMinuteForExtra,
        const Team& home, const Team& away,
        double homeAttack, double homeDefense,
        double awayAttack, double awayDefense,
        double homeContextMod, double awayContextMod,
        int& currentHomeGoals, int& currentAwayGoals,
        MatchLog& log, Random& rng
    );

    void simulatePenaltyShootout(const Team& home, const Team& away, MatchLog& log, Random& rng);

public:
    // Не зберігає стану: уся випадковість приходить через rng
    MatchLog simulateMatch(
        const Team& home, const Team& away, MatchRules rules, Random& rng,
        bool isSecondLeg = false, int firstLegHomeGoals = 0, int firstLegAwayGoals = 0,
        bool neutralVenue = false
    );
};
