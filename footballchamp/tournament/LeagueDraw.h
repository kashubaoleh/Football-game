//LeagueDraw.h
#pragma once
#include "../core/Team.h"
#include "../core/Match.h"
#include "../core/Config.h"
#include "../sim/Random.h"
#include <vector>
#include <string>
#include <utility>

struct Round {
    int roundNumber;
    std::vector<Match> matches;
};

class LeagueDraw {
private:
    std::vector<std::vector<Team>> pots;

    std::pair<int, int> parseCode(const std::string& code) const;

public:
    LeagueDraw(const std::vector<Team>& all36Teams);
    std::vector<Round> generateSchedule(Random& rng);

    // Корзини в порядку жеребкування (до перемішування): 4 корзини по 9 команд
    const std::vector<std::vector<Team>>& getPots() const { return pots; }
};
