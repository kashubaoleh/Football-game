//MatchSimulator.cpp
#include "MatchSimulator.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

// Тег похідного потоку для авторів голів: рахунок і хвилини голів від нього не залежать
constexpr std::uint64_t kScorerStreamTag = 0x5C0DE5;

double scoringWeight(Position position) {
    switch (position) {
        case Position::Goalkeeper: return Config::SCORER_WEIGHT_GOALKEEPER;
        case Position::Defender:   return Config::SCORER_WEIGHT_DEFENDER;
        case Position::Midfielder: return Config::SCORER_WEIGHT_MIDFIELDER;
        case Position::Forward:    return Config::SCORER_WEIGHT_FORWARD;
    }
    return 0.0;
}

// Хто з команди забив: випадковий гравець із вагою за позицією. Порожній рядок, якщо склад невідомий.
std::string pickScorer(const Team& team, Random& rng) {
    std::vector<double> weights;
    weights.reserve(team.squad.size());
    for (const auto& player : team.squad) {
        weights.push_back(scoringWeight(player.position));
    }
    const int index = rng.pickWeighted(weights);
    return index < 0 ? std::string() : team.squad[static_cast<std::size_t>(index)].name;
}

}  // namespace

void MatchSimulator::simulateTimeBlock(
    int startMin, int endMin, int extraMinutes,
    int baseMinuteForExtra,
    const Team& home, const Team& away,
    double homeAttack, double homeDefense,
    double awayAttack, double awayDefense,
    double homeContextMod, double awayContextMod,
    int& currentHomeGoals, int& currentAwayGoals,
    MatchLog& log, Random& rng
) {
    double baseBlockXG = 1.35 / 6.0;

    double homeBlockXG = baseBlockXG * std::pow(homeAttack / awayDefense, Config::XG_POWER) * homeContextMod;
    double awayBlockXG = baseBlockXG * std::pow(awayAttack / homeDefense, Config::XG_POWER) * awayContextMod;

    int totalBlockDuration = (endMin - startMin + 1) + extraMinutes;
    double durationFactor = static_cast<double>(totalBlockDuration) / 15.0;

    homeBlockXG *= durationFactor;
    awayBlockXG *= durationFactor;

    int homeGoals = rng.poisson(homeBlockXG);
    int awayGoals = rng.poisson(awayBlockXG);

    for (int i = 0; i < homeGoals; ++i) {
        int minuteOffset = rng.nextInt(0, totalBlockDuration - 1);
        int absMin = startMin + minuteOffset;
        std::string dispMin = (absMin > baseMinuteForExtra)
            ? std::to_string(baseMinuteForExtra) + "+" + std::to_string(absMin - baseMinuteForExtra) + "'"
            : std::to_string(absMin) + "'";

        currentHomeGoals++;
        log.events.push_back({
            absMin, dispMin, home.name,
            0, 0, // Заповнимо точний хронологічний рахунок пізніше після сортування
            0, 0,
            std::string() // автора голу визначимо пізніше
        });
    }

    for (int i = 0; i < awayGoals; ++i) {
        int minuteOffset = rng.nextInt(0, totalBlockDuration - 1);
        int absMin = startMin + minuteOffset;
        std::string dispMin = (absMin > baseMinuteForExtra)
            ? std::to_string(baseMinuteForExtra) + "+" + std::to_string(absMin - baseMinuteForExtra) + "'"
            : std::to_string(absMin) + "'";

        currentAwayGoals++;
        log.events.push_back({
            absMin, dispMin, away.name,
            0, 0, // Заповнимо точний хронологічний рахунок пізніше після сортування
            0, 0,
            std::string() // автора голу визначимо пізніше
        });
    }
}

void MatchSimulator::simulatePenaltyShootout(const Team& home, const Team& away, MatchLog& log, Random& rng) {
    log.homePenaltyGoals = 0;
    log.awayPenaltyGoals = 0;

    double homeChance = std::clamp(0.75 * (home.attack / away.defense), 0.60, 0.90);
    double awayChance = std::clamp(0.75 * (away.attack / home.defense), 0.60, 0.90);

    int attempt = 1;
    while (attempt <= 5) {
        bool homeScored = rng.chance(homeChance);
        if (homeScored) log.homePenaltyGoals++;
        log.penaltyShootout.push_back({home.name, attempt, homeScored});

        if (log.homePenaltyGoals > log.awayPenaltyGoals + (5 - attempt) ||
            log.awayPenaltyGoals > log.homePenaltyGoals + (5 - (attempt - 1))) return;

        bool awayScored = rng.chance(awayChance);
        if (awayScored) log.awayPenaltyGoals++;
        log.penaltyShootout.push_back({away.name, attempt, awayScored});

        if (log.homePenaltyGoals > log.awayPenaltyGoals + (6 - attempt) ||
        log.awayPenaltyGoals > log.homePenaltyGoals + (5 - attempt)) return;

        attempt++;
    }

    while (log.homePenaltyGoals == log.awayPenaltyGoals) {
        bool homeScored = rng.chance(homeChance);
        if (homeScored) log.homePenaltyGoals++;
        log.penaltyShootout.push_back({home.name, attempt, homeScored});

        bool awayScored = rng.chance(awayChance);
        if (awayScored) log.awayPenaltyGoals++;
        log.penaltyShootout.push_back({away.name, attempt, awayScored});

        attempt++;
    }
}

MatchLog MatchSimulator::simulateMatch(
    const Team& home, const Team& away, MatchRules rules, Random& rng,
    bool isSecondLeg, int firstLegHomeGoals, int firstLegAwayGoals, bool neutralVenue
) {
    MatchLog log;
    log.homeTeamName = home.name;
    log.awayTeamName = away.name;
    log.isSecondLeg = isSecondLeg;
    log.firstLegHomeGoals = firstLegHomeGoals;
    log.firstLegAwayGoals = firstLegAwayGoals;

    double homeDayForm = rng.nextReal(Config::DAY_FORM_MIN, Config::DAY_FORM_MAX);
    double awayDayForm = rng.nextReal(Config::DAY_FORM_MIN, Config::DAY_FORM_MAX);

    double homeBonus = neutralVenue ? 1.0 : Config::HOME_BONUS;

    double homeAttack = home.attack * homeBonus * homeDayForm;
    double homeDefense = home.defense * homeBonus * homeDayForm;
    double awayAttack = away.attack * awayDayForm;
    double awayDefense = away.defense * awayDayForm;

    double classDiff = home.getTotalRating() - away.getTotalRating();

    int currentHomeGoals = 0;
    int currentAwayGoals = 0;

    int extra1stHalf = rng.nextInt(0, 5);
    int extra2ndHalf = rng.nextInt(0, 5);
    int extra1stET   = rng.nextInt(0, 3);
    int extra2ndET   = rng.nextInt(0, 3);

    log.stoppage[0] = extra1stHalf;
    log.stoppage[1] = extra2ndHalf;

    for (int block = 0; block < 6; ++block) {
        int startMin = block * 15 + 1;
        int endMin = (block + 1) * 15;

        double homeContextMod = 1.0;
        double awayContextMod = 1.0;

        int aggHome = currentHomeGoals + firstLegHomeGoals;
        int aggAway = currentAwayGoals + firstLegAwayGoals;
        int aggGoalDiff = aggHome - aggAway;

        if (block >= 3) {
            if (aggGoalDiff < 0 && aggGoalDiff >= -2) {
                homeContextMod *= (classDiff < -20.0) ? 1.08 : 1.20;
            } else if (aggGoalDiff > 0 && aggGoalDiff <= 2) {
                awayContextMod *= (classDiff > 20.0) ? 1.08 : 1.20;
            } else if (std::abs(aggGoalDiff) >= 3) {
                homeContextMod *= 0.85;
                awayContextMod *= 0.85;
            }
        }

        int extraMins = (block == 2) ? extra1stHalf : ((block == 5) ? extra2ndHalf : 0);
        int baseMinForExtra = endMin;

        simulateTimeBlock(
            startMin, endMin, extraMins, baseMinForExtra,
            home, away, homeAttack, homeDefense, awayAttack, awayDefense,
            homeContextMod, awayContextMod, currentHomeGoals, currentAwayGoals,
            log, rng
        );
    }

    log.homeGoals = currentHomeGoals;
    log.awayGoals = currentAwayGoals;
    log.aggHomeGoals = currentHomeGoals + firstLegHomeGoals;
    log.aggAwayGoals = currentAwayGoals + firstLegAwayGoals;
    log.isPlayed = true;

    if (rules == MatchRules::Knockout && log.aggHomeGoals == log.aggAwayGoals) {
        log.extraTimePlayed = true;
        log.stoppage[2] = extra1stET;
        log.stoppage[3] = extra2ndET;
        simulateTimeBlock(
            91, 105, extra1stET, 105, home, away,
            homeAttack, homeDefense, awayAttack, awayDefense,
            1.0, 1.0, currentHomeGoals, currentAwayGoals,
            log, rng
        );

        simulateTimeBlock(
            106, 120, extra2ndET, 120, home, away,
            homeAttack, homeDefense, awayAttack, awayDefense,
            1.0, 1.0, currentHomeGoals, currentAwayGoals,
            log, rng
        );

        log.homeGoals = currentHomeGoals;
        log.awayGoals = currentAwayGoals;
        log.aggHomeGoals = currentHomeGoals + firstLegHomeGoals;
        log.aggAwayGoals = currentAwayGoals + firstLegAwayGoals;

        if (log.aggHomeGoals == log.aggAwayGoals) {
            simulatePenaltyShootout(home, away, log, rng);
        }
    }

    // 1. Спочатку сортуємо за хвилинами
    std::stable_sort(log.events.begin(), log.events.end(), [](const GoalEvent& a, const GoalEvent& b) {
        return goalEventSortKey(a) < goalEventSortKey(b);
    });

    // 2. ТЕПЕР послідовно рахуємо та оновлюємо рахунок у хронологічному порядку
    int runningHomeScore = 0;
    int runningAwayScore = 0;
    for (auto& ev : log.events) {
        if (ev.teamName == home.name) {
            runningHomeScore++;
        } else {
            runningAwayScore++;
        }
        ev.matchHomeScore = runningHomeScore;
        ev.matchAwayScore = runningAwayScore;
        ev.aggHomeScore = runningHomeScore + firstLegHomeGoals;
        ev.aggAwayScore = runningAwayScore + firstLegAwayGoals;
    }

    // 3. Автори голів. Окремий потік випадкових чисел: рахунок і хвилини голів від нього не залежать,
    //    тож той самий seed дає ті самі голи, що й до появи авторів
    Random scorerRng = rng.derive(kScorerStreamTag);
    for (auto& ev : log.events) {
        ev.scorerName = pickScorer(ev.teamName == home.name ? home : away, scorerRng);
    }

    return log;
}
