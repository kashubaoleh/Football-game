//LeagueTable.cpp
#include "LeagueTable.h"
#include <algorithm>

void LeagueTable::initialize(const std::vector<Team>& teams) {
    table.clear();
    table.reserve(teams.size());

    for (const auto& team : teams) {
        TeamStats stats;
        stats.teamName = team.name;
        stats.initialRating = team.initialRating;
        table.push_back(stats);
    }

    sortTable();
}

void LeagueTable::updateMatch(const std::string& homeTeamName, int homeGoals,
                             const std::string& awayTeamName, int awayGoals) {
    TeamStats* home = nullptr;
    TeamStats* away = nullptr;

    for (auto& stats : table) {
        if (stats.teamName == homeTeamName) home = &stats;
        if (stats.teamName == awayTeamName) away = &stats;
    }

    if (!home || !away) return;

    // Оновлюємо кількість зіграних матчів
    home->matchesPlayed++;
    away->matchesPlayed++;

    // Голи
    home->goalsFor += homeGoals;
    home->goalsAgainst += awayGoals;

    away->goalsFor += awayGoals;
    away->goalsAgainst += homeGoals;

    // Результат і очки
    if (homeGoals > awayGoals) {
        home->wins++;
        home->points += 3;
        away->losses++;
    } else if (homeGoals < awayGoals) {
        away->wins++;
        away->points += 3;
        home->losses++;
    } else {
        home->draws++;
        home->points += 1;
        away->draws++;
        away->points += 1;
    }

    // Оновлюємо ранжування команд у таблиці
    sortTable();
}

void LeagueTable::sortTable() {
    std::sort(table.begin(), table.end(), [](const TeamStats& a, const TeamStats& b) {
        // 1. Очки
        if (a.points != b.points)
            return a.points > b.points;

        // 2. Краща різниця голів
        if (a.getGoalDifference() != b.getGoalDifference())
            return a.getGoalDifference() > b.getGoalDifference();

        // 3. Більше забитих голів загалом
        if (a.goalsFor != b.goalsFor)
            return a.goalsFor > b.goalsFor;

        // 4. Більша кількість перемог
        if (a.wins != b.wins)
            return a.wins > b.wins;

        // 5. Вищий рейтинг (менше число initialRating = вищий рейтинг)
        return a.initialRating < b.initialRating;
    });
}
