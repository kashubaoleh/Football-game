// PlayoffManager.h
#pragma once
#include <vector>
#include <string>
#include "../core/Team.h"
#include "../core/Match.h"
#include "LeagueTable.h"
#include "../sim/Random.h"

struct PlayoffRound {
    int roundIndex; // 0: 1/16, 1: 1/8, 2: 1/4, 3: 1/2, 4: Final
    std::string roundName;
    std::vector<Match> matches;
};

// Пара ще не згенерованої сітки: хто може опинитися на місці господарів / гостей першого матчу.
// Якщо команда вже відома - у списку одна, якщо претендують двоє - у списку дві.
struct DrawPreviewPair {
    std::vector<Team> home;
    std::vector<Team> away;
};

class PlayoffManager {
private:
    std::vector<PlayoffRound> playoffRounds;
    int currentPlayoffRound = 0; // Поточний раунд плей-оф
    int currentPlayoffMatch = 0; // Поточний матч у раунді

    // Допоміжні структури для збереження прохідних команд між раундами
    std::vector<Team> roundWinners;
    bool draw18Completed = false; // Чи проведене жеребкування 1/8

    // Створює матчі раунду за парами: спершу перші матчі, потім матчі-відповіді (поля міняються)
    void fillTwoLeggedRound(int roundIdx, const std::vector<std::pair<Team, Team>>& pairs);
public:
    PlayoffManager();

    Team findTeamByRank(const std::vector<TeamStats>& table, const std::vector<Team>& allTeams, int rank) const;
    // Генерація раундів
    void generateRoundOf16(const std::vector<TeamStats>& finalLeagueTable, const std::vector<Team>& allTeams, Random& rng);
    void generateRoundOf8(const std::vector<Team>& top8Teams, Random& rng);

    // Сітка, яка ще не згенерована: для кожної пари першого матчу - претенденти на кожне місце.
    // Порядок пар збігається з порядком перших матчів після жеребкування.
    std::vector<DrawPreviewPair> previewRoundOf16(const std::vector<TeamStats>& finalLeagueTable,
                                                  const std::vector<Team>& allTeams) const;
    std::vector<DrawPreviewPair> previewRoundOf8(const std::vector<Team>& top8Teams) const;
    void generateQuarterFinals();
    void generateSemiFinals();
    void generateFinal();

    bool isPlayoffFinished() const;

    PlayoffRound& getCurrentRound();
    const PlayoffRound& getCurrentRound() const;

    int getCurrentRoundIndex() const { return currentPlayoffRound; }
    int getCurrentMatchIndex() const { return currentPlayoffMatch; }

    void advanceMatch();

    // Отримати всі раунди для відображення в календарі
    const std::vector<PlayoffRound>& getAllRounds() const { return playoffRounds; }

    void addWinner(const Team& team) { roundWinners.push_back(team); }
    void clearWinners() { roundWinners.clear(); }
    const std::vector<Team>& getWinners() const { return roundWinners; }

    bool isDraw18Completed() const { return draw18Completed; }
};
