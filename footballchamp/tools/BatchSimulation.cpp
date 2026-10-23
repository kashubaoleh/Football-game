// BatchSimulation.cpp
#include "BatchSimulation.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include "../core/Config.h"
#include "../tournament/Tournament.h"

void runBatchSimulation(const std::vector<Team>& teams, int runs, std::uint32_t seed) {
    struct Stat {
        int titles = 0, finals = 0, top8 = 0, top24 = 0;
        long long rankSum = 0;
    };
    std::map<std::string, Stat> stats;
    int completed = 0;
    std::cout << "Seed: " << seed << "\n";

    for (int i = 0; i < runs; ++i) {
        Tournament tournament(teams, seed);
        seed = Random::nextSeed(seed);
        TournamentResult r = tournament.runSilentTournament();

        if (r.champion.empty()) continue; // турнір не завершився, пропускаємо
        ++completed;

        stats[r.champion].titles++;
        stats[r.finalist].finals++;
        stats[r.champion].finals++;

        for (const auto& kv : r.leagueRank) {
            Stat& s = stats[kv.first];
            s.rankSum += kv.second;
            if (kv.second <= 8)  s.top8++;
            if (kv.second <= 24) s.top24++;
        }

        if ((i + 1) % 100 == 0) {
            std::cout << "Виконано " << (i + 1) << " / " << runs << "\n";
        }
    }

    if (completed == 0) {
        std::cout << "Жоден турнір не завершився коректно.\n";
        return;
    }

    // Сортуємо команди за кількістю перемог
    std::vector<std::pair<std::string, Stat>> sorted(stats.begin(), stats.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        if (a.second.titles != b.second.titles) return a.second.titles > b.second.titles;
        return a.second.rankSum < b.second.rankSum;
    });

    std::cout << "\n===== РЕЗУЛЬТАТИ " << completed << " ТУРНІРІВ =====\n";
    std::cout << "Команда | Перемоги% | Фінали% | Сер.місце | Топ-8% | Топ-24%\n";
    std::cout << std::fixed << std::setprecision(1);
    for (const auto& p : sorted) {
        const Stat& s = p.second;
        std::cout << p.first << " | "
                  << 100.0 * s.titles / completed << " | "
                  << 100.0 * s.finals / completed << " | "
                  << static_cast<double>(s.rankSum) / completed << " | "
                  << 100.0 * s.top8 / completed << " | "
                  << 100.0 * s.top24 / completed << "\n";
    }

    // Підсумок по потах (за порядком у векторі teams: по 9 команд у поті)
    std::cout << "\n===== ПО ПОТАХ =====\n";
    std::cout << "Пот | Сер.місце | Частка перемог% | У топ-8% | У топ-24%\n";
    for (int pot = 0; pot < 4; ++pot) {
        double rankSum = 0, titles = 0, top8 = 0, top24 = 0;
        for (int k = 0; k < Config::TEAMS_PER_POT; ++k) {
            size_t idx = pot * Config::TEAMS_PER_POT + k;
            if (idx >= teams.size()) break;
            const Stat& s = stats[teams[idx].name];
            rankSum += s.rankSum; titles += s.titles; top8 += s.top8; top24 += s.top24;
        }
        std::cout << (pot + 1) << "   | "
                  << rankSum / (Config::TEAMS_PER_POT * completed) << " | "
                  << 100.0 * titles / completed << " | "
                  << 100.0 * top8 / (Config::TEAMS_PER_POT * completed) << " | "
                  << 100.0 * top24 / (Config::TEAMS_PER_POT * completed) << "\n";
    }
}
