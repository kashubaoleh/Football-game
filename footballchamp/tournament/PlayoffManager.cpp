// PlayoffManager.cpp
#include "PlayoffManager.h"
#include <algorithm>

namespace {

// Структура жеребкування 1/16: кожен рядок - місця в таблиці ліги {r1, r2, r3, r4}.
// Одна з команд r1/r2 грає з однією з команд r3/r4, друга з r1/r2 - з тією, що залишилась.
// Команди з r1/r2 приймають перший матч.
constexpr int kRoundOf16Groups[4][4] = {
    {21, 22, 11, 12},
    {19, 20, 13, 14},
    {23, 24,  9, 10},
    {17, 18, 15, 16}
};

// Структура жеребкування 1/8: місця в таблиці ліги, з яких переможці 1/16 отримують суперників.
// Переможець пари i грає з однією з команд kRoundOf8Pools[i % 4]; пари i та i + 4 ділять цей пул.
constexpr int kRoundOf8Pools[4][2] = {
    {5, 6}, {3, 4}, {7, 8}, {1, 2}
};

// Пари сусідніх переможців: (0,1), (2,3), ...
std::vector<std::pair<Team, Team>> pairNeighbours(const std::vector<Team>& winners) {
    std::vector<std::pair<Team, Team>> pairs;
    for (size_t i = 0; i < winners.size(); i += 2) {
        pairs.push_back({winners[i], winners[i + 1]});
    }
    return pairs;
}

} // namespace

PlayoffManager::PlayoffManager() {
    // Ініціалізуємо порожні раунди (1/16, 1/8, 1/4, 1/2, Фінал)
    playoffRounds.resize(5);
    playoffRounds[0].roundIndex = 0; playoffRounds[0].roundName = "1/16 фіналу";
    playoffRounds[1].roundIndex = 1; playoffRounds[1].roundName = "1/8 фіналу";
    playoffRounds[2].roundIndex = 2; playoffRounds[2].roundName = "Чвертьфінал";
    playoffRounds[3].roundIndex = 3; playoffRounds[3].roundName = "Півфінал";
    playoffRounds[4].roundIndex = 4; playoffRounds[4].roundName = "Фінал";
}

// Допоміжна функція для пошуку команди за місцем в таблиці
Team PlayoffManager::findTeamByRank(const std::vector<TeamStats>& table, const std::vector<Team>& allTeams, int rank) const {
    std::string name = table[rank - 1].teamName;
    for (const auto& t : allTeams) {
        if (t.name == name) return t;
    }
    return allTeams[0];
}

void PlayoffManager::fillTwoLeggedRound(int roundIdx, const std::vector<std::pair<Team, Team>>& pairs) {
    auto& matches = playoffRounds[roundIdx].matches;
    matches.clear();

    // 1. Перші матчі
    for (const auto& p : pairs) {
        Match m;
        m.homeTeam = p.first;
        m.awayTeam = p.second;
        m.isPlayed = false;
        matches.push_back(m);
    }
    // 2. Матчі-відповіді (команди міняються місцями)
    for (const auto& p : pairs) {
        Match m;
        m.homeTeam = p.second;
        m.awayTeam = p.first;
        m.isPlayed = false;
        matches.push_back(m);
    }
}

std::vector<DrawPreviewPair> PlayoffManager::previewRoundOf16(const std::vector<TeamStats>& finalLeagueTable,
                                                              const std::vector<Team>& allTeams) const {
    std::vector<DrawPreviewPair> pairs;
    for (const auto& g : kRoundOf16Groups) {
        DrawPreviewPair p;
        p.home = {findTeamByRank(finalLeagueTable, allTeams, g[0]), findTeamByRank(finalLeagueTable, allTeams, g[1])};
        p.away = {findTeamByRank(finalLeagueTable, allTeams, g[2]), findTeamByRank(finalLeagueTable, allTeams, g[3])};
        pairs.push_back(p);
    }
    // Другі чотири пари кожної групи мають тих самих претендентів (беруть тих, що залишились)
    const size_t groups = pairs.size();
    for (size_t i = 0; i < groups; ++i) {
        pairs.push_back(pairs[i]);
    }
    return pairs;
}

std::vector<DrawPreviewPair> PlayoffManager::previewRoundOf8(const std::vector<Team>& top8Teams) const {
    std::vector<DrawPreviewPair> pairs;
    for (size_t i = 0; i < roundWinners.size(); ++i) {
        const auto& pool = kRoundOf8Pools[i % 4];
        DrawPreviewPair p;
        p.home = {roundWinners[i]};
        p.away = {top8Teams[pool[0] - 1], top8Teams[pool[1] - 1]};
        pairs.push_back(p);
    }
    return pairs;
}

void PlayoffManager::generateRoundOf16(const std::vector<TeamStats>& finalLeagueTable, const std::vector<Team>& allTeams, Random& rng) {
    std::vector<std::pair<Team, Team>> selectedPairs; // перша пара кожної групи
    std::vector<std::pair<Team, Team>> leftPairs;     // друга пара кожної групи (з команд, що залишились)

    for (const auto& g : kRoundOf16Groups) {
        std::vector<int> groupA = {g[0], g[1]};
        std::vector<int> groupB = {g[2], g[3]};
        rng.shuffle(groupA);
        rng.shuffle(groupB);

        selectedPairs.push_back({findTeamByRank(finalLeagueTable, allTeams, groupA[0]),
                                 findTeamByRank(finalLeagueTable, allTeams, groupB[0])});
        leftPairs.push_back({findTeamByRank(finalLeagueTable, allTeams, groupA[1]),
                             findTeamByRank(finalLeagueTable, allTeams, groupB[1])});
    }

    std::vector<std::pair<Team, Team>> matchPairs = selectedPairs;
    matchPairs.insert(matchPairs.end(), leftPairs.begin(), leftPairs.end());

    fillTwoLeggedRound(0, matchPairs);
}

void PlayoffManager::generateRoundOf8(const std::vector<Team>& top8Teams, Random& rng) {
    // Для кожного пулу випадково визначаємо, яка з двох команд дістається першій парі
    std::vector<Team> firstOpponents;  // суперники для переможців 0..3
    std::vector<Team> secondOpponents; // суперники для переможців 4..7
    for (const auto& pool : kRoundOf8Pools) {
        const Team& t1 = top8Teams[pool[0] - 1];
        const Team& t2 = top8Teams[pool[1] - 1];
        const bool firstGetsT1 = rng.nextInt(0, 1) == 0;
        firstOpponents.push_back(firstGetsT1 ? t1 : t2);
        secondOpponents.push_back(firstGetsT1 ? t2 : t1);
    }

    std::vector<std::pair<Team, Team>> matchPairs;
    for (size_t i = 0; i < 8; ++i) {
        matchPairs.push_back({roundWinners[i], i < 4 ? firstOpponents[i] : secondOpponents[i - 4]});
    }

    fillTwoLeggedRound(1, matchPairs);

    draw18Completed = true;
    roundWinners.clear();
}

void PlayoffManager::generateQuarterFinals() {
    fillTwoLeggedRound(2, pairNeighbours(roundWinners));
    roundWinners.clear();
}

void PlayoffManager::generateSemiFinals() {
    fillTwoLeggedRound(3, pairNeighbours(roundWinners));
    roundWinners.clear();
}

void PlayoffManager::generateFinal() {
    // Фінал — це ОДИН матч (тут не потрібні матчі-відповіді)
    playoffRounds[4].matches.clear();
    if (roundWinners.size() >= 2) {
        Match m;
        m.homeTeam = roundWinners[0];
        m.awayTeam = roundWinners[1];
        m.isPlayed = false;
        playoffRounds[4].matches.push_back(m);
    }
}

bool PlayoffManager::isPlayoffFinished() const {
    if (currentPlayoffRound >= 5) return true;
    return false;
}

PlayoffRound& PlayoffManager::getCurrentRound() {
    return playoffRounds[currentPlayoffRound];
}

const PlayoffRound& PlayoffManager::getCurrentRound() const {
    return playoffRounds[currentPlayoffRound];
}

void PlayoffManager::advanceMatch() {
    currentPlayoffMatch++;
    if (currentPlayoffMatch >= static_cast<int>(playoffRounds[currentPlayoffRound].matches.size())) {
        currentPlayoffMatch = 0;
        currentPlayoffRound++;
    }
}
