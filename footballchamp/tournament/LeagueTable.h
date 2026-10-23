//LeagueTable.h
#pragma once
#include <string>
#include <vector>
#include "../core/Team.h"

struct TeamStats {
    std::string teamName;
    int initialRating = 0;

    int matchesPlayed = 0;
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int points = 0;

    int getGoalDifference() const {
        return goalsFor - goalsAgainst;
    }
};

class LeagueTable {
private:
    std::vector<TeamStats> table;

    // Внутрішній метод для сортування за 5 правилами
    void sortTable();

public:
    LeagueTable() = default;

    // Ініціалізація 36 команд
    void initialize(const std::vector<Team>& teams);

    // Автоматичне оновлення за наслідками матчу
    void updateMatch(const std::string& homeTeamName, int homeGoals,
                     const std::string& awayTeamName, int awayGoals);


    const std::vector<TeamStats>& getTable() const { return table; }
};
