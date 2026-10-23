// GameEngine.cpp - головне меню
#include "GameEngine.h"
#include <iostream>
#include "Console.h"
#include "Screens.h"

namespace {

void printNextAction(const Tournament& tournament) {
    switch (tournament.nextAction()) {
        case NextAction::LeagueDraw:
            std::cout << " 1. Наступна дія: [Жеребкування Етапу Ліги]\n";
            break;

        case NextAction::LeagueMatch: {
            const int roundIdx = tournament.getLeagueRoundIndex();
            const int matchIdx = tournament.getLeagueMatchIndex();
            const auto& nextMatch = tournament.getSchedule()[roundIdx].matches[matchIdx];
            std::cout << " 1. Наступна дія: [Тур " << (roundIdx + 1)
                      << " | Матч " << (matchIdx + 1) << "/18: "
                      << nextMatch.homeTeam.name << " vs " << nextMatch.awayTeam.name << "]\n";
            break;
        }

        case NextAction::RoundOf16Draw:
            std::cout << " 1. Наступна дія: [Жеребкування 1/16 фіналу]\n";
            break;

        case NextAction::RoundOf8Draw:
            std::cout << " 1. Наступна дія: [Жеребкування 1/8 фіналу]\n";
            break;

        case NextAction::PlayoffMatch: {
            const auto& curRound = tournament.getPlayoff().getCurrentRound();
            int matchIdx = tournament.getPlayoff().getCurrentMatchIndex();
            const auto& nextMatch = curRound.matches[matchIdx];

            std::cout << " 1. Наступна дія: [" << curRound.roundName
                      << " | Матч " << (matchIdx + 1) << "/" << curRound.matches.size()
                      << ": " << nextMatch.homeTeam.name << " vs " << nextMatch.awayTeam.name;

            // Матч-відповідь (усі раунди, крім фіналу): рахунок першого матчу
            size_t half = curRound.matches.size() / 2;
            bool isSecondLeg = (curRound.roundIndex != 4) && (static_cast<size_t>(matchIdx) >= half);

            if (isSecondLeg) {
                const auto& firstLeg = curRound.matches[matchIdx - half];
                // Порядок як у поточному матчі: ліве число належить господарям (Б), праве гостям (А)
                std::cout << " [" << firstLeg.resultLog.awayGoals << " - "
                          << firstLeg.resultLog.homeGoals << "]";
            }

            std::cout << "]\n";
            break;
        }

        case NextAction::NewTournament:
            std::cout << " 1. Наступна дія: [Запустити наступний турнір]\n";
            break;
    }
}

} // namespace

GameEngine::GameEngine(const std::vector<Team>& teams, std::uint32_t seed) : tournament(teams, seed) {}

void GameEngine::run() {
    bool isRunning = true;

    while (isRunning) {
        ui::clearScreen();
        std::cout << "=======================================================\n";
        std::cout << "              ГОЛОВНЕ МЕНЮ ТУРНІРУ                     \n";
        std::cout << "=======================================================\n\n";
        std::cout << " Seed турніру: " << tournament.getSeed() << "\n\n";

        printNextAction(tournament);

        std::cout << " 2. Календар\n";
        std::cout << " 3. Таблиця\n";
        std::cout << " 4. Вийти\n";
        std::cout << "\nВиберіть пункт (1-4): ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1:
                switch (tournament.nextAction()) {
                    case NextAction::LeagueDraw:
                        ui::showDrawMenu(tournament);
                        break;
                    case NextAction::LeagueMatch:
                    case NextAction::PlayoffMatch:
                        ui::showMatchPreviewMenu(tournament);
                        break;
                    case NextAction::RoundOf16Draw:
                        ui::showPlayoffDrawMenu(tournament); // Жеребкування 1/16
                        break;
                    case NextAction::RoundOf8Draw:
                        ui::showPlayoffDraw18Menu(tournament); // Жеребкування 1/8
                        break;
                    case NextAction::NewTournament:
                        ui::showNextTournamentMenu(tournament);
                        break;
                }
                break;
            case 2:
                ui::showCalendar(tournament);
                break;
            case 3:
                ui::showLeagueTable(tournament.getLeagueTable());
                break;
            case 4:
                isRunning = false;
                std::cout << "\nДякуємо за гру! До зустрічі!\n";
                break;
            default:
                break;
        }
    }
}
