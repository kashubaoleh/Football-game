// MatchLog.h
#pragma once
#include <string>
#include <vector>

enum class MatchRules { GroupStage, Knockout };

struct GoalEvent {
    int absoluteMinute;
    std::string displayMinute;
    std::string teamName;
    int matchHomeScore;
    int matchAwayScore;
    int aggHomeScore;
    int aggAwayScore;
    std::string scorerName;  // хто забив (порожньо, якщо склад команди невідомий)
};

// Ключ сортування: "45'" -> 4500, "45+1'" -> 4501, "46'" -> 4600
inline int goalEventSortKey(const GoalEvent& e) {
    size_t plus = e.displayMinute.find('+');
    if (plus == std::string::npos) return std::stoi(e.displayMinute) * 100;
    return std::stoi(e.displayMinute.substr(0, plus)) * 100
         + std::stoi(e.displayMinute.substr(plus + 1));
}

struct PenaltyEvent {
    std::string teamName;
    int attemptNumber;
    bool isScored;
};

struct MatchLog {
    std::string homeTeamName;
    std::string awayTeamName;

    int homeGoals = 0;
    int awayGoals = 0;

    bool isPlayed = false;

    bool isSecondLeg = false;
    int firstLegHomeGoals = 0;
    int firstLegAwayGoals = 0;

    int aggHomeGoals = 0;
    int aggAwayGoals = 0;

    int homePenaltyGoals = -1;
    int awayPenaltyGoals = -1;

    bool extraTimePlayed = false;
    int stoppage[4] = {0, 0, 0, 0}; // 0: 1-й тайм, 1: 2-й тайм, 2: ДЧ 1, 3: ДЧ 2

    std::vector<GoalEvent> events;
    std::vector<PenaltyEvent> penaltyShootout;
};
