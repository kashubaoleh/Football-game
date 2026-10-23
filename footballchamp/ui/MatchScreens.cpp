// MatchScreens.cpp - афіша, швидкий результат і трансляція матчів ліги та плей-оф
#include "Screens.h"
#include <iomanip>
#include <iostream>
#include "../core/Config.h"
#include "Broadcast.h"
#include "Console.h"
#include "Printers.h"

namespace ui {

namespace {

void showPlayoffMatchPlayback(const Match& match, int speedMode) {
    clearScreen();

    if (speedMode == 1) {
        printMatchHeader(match);
        printQuickResult(match);
        if (match.resultLog.extraTimePlayed) {
            std::cout << "\n (Матч тривав додатковий час)\n";
        }
    } else {
        playMatchBroadcast(match, delayForSpeed(speedMode));
    }
    std::cout << "\n";
}

} // namespace

void processMatchSimulation(Tournament& tournament, int speedMode) {
    const Match& match = tournament.playNextLeagueMatch();

    clearScreen();
    printMatchHeader(match);

    if (speedMode == 1) {
        printQuickResult(match);
    } else {
        playMatchBroadcast(match, delayForSpeed(speedMode));
    }

    waitEnter();
}

void processPlayoffMatchSimulation(Tournament& tournament, int speedMode) {
    PlayoffStepResult res = tournament.playCurrentPlayoffMatch();
    const Match& match = *res.match;

    showPlayoffMatchPlayback(match, speedMode);

    switch (res.kind) {
        case PlayoffStepResult::Kind::Final: {
            std::cout << "=======================================================\n";
            std::cout << "              ФІНАЛ ЛІГИ ЧЕМПІОНІВ                     \n";
            std::cout << "=======================================================\n";
            std::cout << match.homeTeam.name << "  " << match.resultLog.homeGoals << " - "
                      << match.resultLog.awayGoals << "  " << match.awayTeam.name << "\n";

            if (res.decidedByPenalties) {
                std::cout << "Серія пенальті: " << match.resultLog.homePenaltyGoals << " - "
                          << match.resultLog.awayPenaltyGoals << "\n";
            }
            std::cout << "\n ПЕРЕМОЖЕЦЬ ЛІГИ ЧЕМПІОНІВ: " << res.winner.name << "!\n";
            break;
        }

        case PlayoffStepResult::Kind::FirstLeg: {
            std::cout << "=======================================================\n";
            std::cout << " ПЕРШИЙ МАТЧ (" << res.roundName << ")\n";
            std::cout << "=======================================================\n";
            std::cout << match.homeTeam.name << "  " << match.resultLog.homeGoals << " - "
                      << match.resultLog.awayGoals << "  " << match.awayTeam.name << "\n\n";
            std::cout << "[Перший матч завершено. У матчі-відповіді команди поміняються полями].\n";
            break;
        }

        case PlayoffStepResult::Kind::SecondLeg: {
            const MatchLog& secondLegLog = match.resultLog;

            std::cout << "=======================================================\n";
            std::cout << " МАТЧ-ВІДПОВІДЬ ТА ПІДСУМОК ДУЕЛІ (" << res.roundName << ")\n";
            std::cout << "=======================================================\n";
            std::cout << "Матч-відповідь: " << match.homeTeam.name << " " << secondLegLog.homeGoals << " - "
                      << secondLegLog.awayGoals << " " << match.awayTeam.name << "\n";
            std::cout << "Загальний рахунок (Agg): " << match.homeTeam.name << " [" << res.homeAggregate
                      << " - " << res.awayAggregate << "] " << match.awayTeam.name << "\n";

            if (res.decidedByPenalties) {
                std::cout << "Серія пенальті!\n";
                std::cout << match.homeTeam.name << " " << secondLegLog.homePenaltyGoals << " - "
                          << secondLegLog.awayPenaltyGoals << " " << match.awayTeam.name << "\n";
            }

            std::cout << "\n У наступний раунд виходить: " << res.winner.name << "!\n";

            if (res.generatedRoundIndex == 2) {
                std::cout << "\n[ Усі матчі 1/8 завершено! Сітку Чвертьфіналів сформовано! ]\n";
            } else if (res.generatedRoundIndex == 3) {
                std::cout << "\n[ Усі матчі 1/4 завершено! Сітку Півфіналів сформовано! ]\n";
            } else if (res.generatedRoundIndex == 4) {
                std::cout << "\n[ Усі матчі Півфіналу завершено! Учасників Фіналу визначено! ]\n";
            }
            break;
        }
    }

    waitEnter();
}

void showMatchPreviewMenu(Tournament& tournament) {
    Team homeTeam, awayTeam;
    std::string tournamentStageStr;

    if (tournament.getLeagueRoundIndex() < Config::LEAGUE_ROUNDS) {
        // Етап ліги
        const Match& currentMatch = tournament.getSchedule()[tournament.getLeagueRoundIndex()].matches[tournament.getLeagueMatchIndex()];
        homeTeam = currentMatch.homeTeam;
        awayTeam = currentMatch.awayTeam;
        tournamentStageStr = "Етап Ліги (Тур " + std::to_string(tournament.getLeagueRoundIndex() + 1) + " | Матч " + std::to_string(tournament.getLeagueMatchIndex() + 1) + " з 18)";
    } else {
        // Плей-оф
        auto& curRound = tournament.getPlayoff().getCurrentRound();
        const Match& currentMatch = curRound.matches[tournament.getPlayoff().getCurrentMatchIndex()];
        homeTeam = currentMatch.homeTeam;
        awayTeam = currentMatch.awayTeam;
        tournamentStageStr = curRound.roundName + " (Матч " + std::to_string(tournament.getPlayoff().getCurrentMatchIndex() + 1) + "/" + std::to_string(curRound.matches.size()) + ")";
    }

    clearScreen();
    std::cout << "=======================================================\n";
    std::cout << "                     АФІША МАТЧУ                       \n";
    std::cout << "=======================================================\n";
    std::cout << " Турнір: " << tournamentStageStr << "\n";
    std::cout << " МАТЧ:   " << homeTeam.name << "  VS  " << awayTeam.name << "\n";
    std::cout << "=======================================================\n\n";
    std::cout << " 1. Швидко показати результат\n";
    std::cout << " 2. Швидка трансляція\n";
    std::cout << " 3. Середня трансляція\n";
    std::cout << " 4. Повільна трансляція\n";
    std::cout << " 5. Повернутися в меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;

    if (choice >= 1 && choice <= 4) {
        if (tournament.getLeagueRoundIndex() < Config::LEAGUE_ROUNDS) {
            processMatchSimulation(tournament, choice); // Звичайна симуляція лігового матчу
        } else {
            processPlayoffMatchSimulation(tournament, choice); // Симуляція матчу плей-оф (з урахуванням двоматчевих дуелей)
        }
    }
}

}
