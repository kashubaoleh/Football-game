//LeagueDraw.cpp
#include "LeagueDraw.h"
#include <algorithm>

LeagueDraw::LeagueDraw(const std::vector<Team>& all36Teams) {
    pots.resize(4);
    for (size_t i = 0; i < Config::TEAMS_COUNT; ++i) {
        pots[i / Config::TEAMS_PER_POT].push_back(all36Teams[i]);
    }
}

std::pair<int, int> LeagueDraw::parseCode(const std::string& code) const {
    int pot = code[1] - '1';
    int team = code[4] - '1';
    return {pot, team};
}

std::vector<Round> LeagueDraw::generateSchedule(Random& rng) {
    for (int i = 0; i < 4; ++i) {
        rng.shuffle(pots[i]);
    }

    std::vector<std::vector<std::pair<std::string, std::string>>> rawTemplate = {
        // Round 1
        {{"P4_T6", "P1_T7"}, {"P4_T5", "P1_T6"}, {"P1_T5", "P3_T5"}, {"P2_T3", "P2_T4"}, {"P3_T2", "P3_T3"},
         {"P4_T2", "P4_T3"}, {"P2_T1", "P4_T1"}, {"P4_T9", "P3_T1"}, {"P3_T9", "P1_T1"}, {"P1_T2", "P2_T2"},
         {"P1_T3", "P1_T4"}, {"P3_T4", "P4_T4"}, {"P2_T5", "P2_T6"}, {"P3_T6", "P2_T7"}, {"P3_T7", "P1_T8"},
         {"P3_T8", "P2_T9"}, {"P2_T8", "P1_T9"}, {"P4_T7", "P4_T8"}},
        // Round 2
        {{"P2_T9", "P4_T9"}, {"P1_T1", "P3_T1"}, {"P4_T8", "P3_T9"}, {"P1_T8", "P1_T9"}, {"P2_T8", "P3_T8"},
         {"P3_T7", "P4_T7"}, {"P1_T7", "P2_T7"}, {"P4_T5", "P4_T6"}, {"P4_T4", "P1_T5"}, {"P1_T6", "P3_T6"},
         {"P3_T5", "P2_T6"}, {"P2_T4", "P2_T5"}, {"P4_T3", "P1_T4"}, {"P3_T3", "P3_T4"}, {"P1_T3", "P2_T3"},
         {"P2_T2", "P4_T2"}, {"P4_T1", "P3_T2"}, {"P2_T1", "P1_T2"}},
        // Round 3
        {{"P2_T4", "P3_T4"}, {"P4_T3", "P4_T4"}, {"P3_T5", "P1_T6"}, {"P3_T6", "P4_T6"}, {"P2_T7", "P3_T7"},
         {"P2_T6", "P1_T7"}, {"P4_T7", "P2_T8"}, {"P1_T8", "P4_T8"}, {"P3_T8", "P3_T9"}, {"P1_T9", "P4_T9"},
         {"P2_T9", "P1_T1"}, {"P2_T1", "P2_T2"}, {"P3_T1", "P4_T1"}, {"P1_T2", "P4_T2"}, {"P3_T2", "P1_T3"},
         {"P2_T3", "P3_T3"}, {"P1_T4", "P1_T5"}, {"P2_T5", "P4_T5"}},
        // Round 4
        {{"P4_T2", "P1_T3"}, {"P3_T3", "P4_T3"}, {"P2_T2", "P3_T2"}, {"P2_T3", "P1_T4"}, {"P2_T4", "P4_T4"},
         {"P3_T7", "P3_T8"}, {"P2_T6", "P4_T6"}, {"P4_T5", "P3_T6"}, {"P3_T4", "P3_T5"}, {"P1_T5", "P2_T5"},
         {"P1_T6", "P1_T7"}, {"P4_T7", "P1_T8"}, {"P2_T7", "P2_T8"}, {"P4_T8", "P2_T9"}, {"P4_T9", "P4_T1"},
         {"P1_T1", "P1_T2"}, {"P2_T1", "P3_T1"}, {"P1_T9", "P3_T9"}},
        // Round 5
        {{"P3_T4", "P2_T5"}, {"P4_T3", "P2_T4"}, {"P4_T4", "P3_T5"}, {"P1_T6", "P4_T6"}, {"P3_T6", "P3_T7"},
         {"P2_T6", "P2_T7"}, {"P1_T7", "P4_T7"}, {"P2_T8", "P4_T8"}, {"P1_T8", "P3_T8"}, {"P3_T9", "P4_T9"},
         {"P1_T9", "P2_T9"}, {"P1_T1", "P2_T1"}, {"P3_T1", "P2_T2"}, {"P4_T1", "P4_T2"}, {"P1_T2", "P1_T3"},
         {"P3_T2", "P2_T3"}, {"P3_T3", "P1_T4"}, {"P1_T5", "P4_T5"}},
        // Round 6
        {{"P1_T4", "P4_T4"}, {"P3_T2", "P4_T2"}, {"P2_T2", "P1_T3"}, {"P2_T3", "P4_T3"}, {"P3_T3", "P2_T4"},
         {"P4_T6", "P3_T7"}, {"P4_T5", "P2_T6"}, {"P3_T5", "P3_T6"}, {"P3_T4", "P1_T5"}, {"P2_T5", "P1_T6"},
         {"P1_T7", "P1_T8"}, {"P2_T7", "P4_T7"}, {"P2_T8", "P2_T9"}, {"P4_T8", "P4_T9"}, {"P1_T1", "P4_T1"},
         {"P3_T1", "P1_T2"}, {"P3_T9", "P2_T1"}, {"P3_T8", "P1_T9"}},
        // Round 7
        {{"P2_T9", "P2_T1"}, {"P4_T8", "P1_T9"}, {"P1_T8", "P2_T8"}, {"P4_T7", "P3_T8"}, {"P1_T7", "P3_T7"},
         {"P2_T6", "P3_T6"}, {"P4_T6", "P2_T7"}, {"P1_T5", "P1_T6"}, {"P4_T4", "P4_T5"}, {"P2_T5", "P3_T5"},
         {"P4_T3", "P3_T4"}, {"P1_T4", "P2_T4"}, {"P1_T3", "P3_T3"}, {"P1_T2", "P3_T2"}, {"P4_T2", "P2_T3"},
         {"P4_T1", "P2_T2"}, {"P3_T9", "P3_T1"}, {"P4_T9", "P1_T1"}},
        // Round 8
        {{"P4_T2", "P3_T3"}, {"P1_T3", "P4_T3"}, {"P3_T6", "P1_T7"}, {"P2_T7", "P1_T8"}, {"P4_T6", "P4_T7"},
         {"P3_T7", "P2_T8"}, {"P2_T9", "P3_T9"}, {"P4_T9", "P2_T1"}, {"P3_T8", "P4_T8"}, {"P1_T9", "P1_T1"},
         {"P4_T1", "P1_T2"}, {"P3_T1", "P3_T2"}, {"P2_T2", "P2_T3"}, {"P1_T4", "P3_T4"}, {"P2_T4", "P1_T5"},
         {"P4_T4", "P2_T5"}, {"P1_T6", "P2_T6"}, {"P3_T5", "P4_T5"}}
    };

    std::vector<Round> schedule;
    for (int r = 0; r < Config::LEAGUE_ROUNDS; ++r) {
        Round round;
        round.roundNumber = r + 1;

        for (const auto& matchPair : rawTemplate[r]) {
            auto homeLoc = parseCode(matchPair.first);
            auto awayLoc = parseCode(matchPair.second);

            Match m;
            m.homeTeam = pots[homeLoc.first][homeLoc.second];
            m.awayTeam = pots[awayLoc.first][awayLoc.second];
            m.isPlayed = false;

            round.matches.push_back(m);
        }

        rng.shuffle(round.matches);
        schedule.push_back(round);
    }

    rng.shuffle(schedule);

    for (int i = 0; i < Config::LEAGUE_ROUNDS; ++i) {
        schedule[i].roundNumber = i + 1;
    }

    return schedule;
}
