// TableScreen.cpp - турнірна таблиця ліги
#include "Screens.h"
#include <iomanip>
#include <iostream>
#include <string>
#include "Console.h"

namespace ui {

void showLeagueTable(const LeagueTable& leagueTable) {
    const auto& table = leagueTable.getTable();
    clearScreen();
    std::cout << "=========================================================================================\n";
    std::cout << "                                  ТУРНІРНА ТАБЛИЦЯ ЛІГИ                                  \n";
    std::cout << "=========================================================================================\n";
    std::cout << std::left << std::setw(4)  << " #"
              << std::left << std::setw(28) << "Команда"
              << std::right << std::setw(5) << "ЗМ"
              << std::right << std::setw(5) << "В"
              << std::right << std::setw(5) << "Н"
              << std::right << std::setw(5) << "П"
              << std::right << std::setw(6) << "ЗГ"
              << std::right << std::setw(6) << "ПГ"
              << std::right << std::setw(7) << "РГ"
              << std::right << std::setw(6) << "О" << "\n";
    std::cout << "-----------------------------------------------------------------------------------------\n";

    for (size_t i = 0; i < table.size(); ++i) {
        const auto& team = table[i];
        int gd = team.getGoalDifference();
        std::string gdStr = (gd > 0 ? "+" : "") + std::to_string(gd);

        std::cout << std::right << std::setw(2) << (i + 1) << " | "
                  << std::left << std::setw(25) << team.teamName
                  << std::right << std::setw(5) << team.matchesPlayed
                  << std::right << std::setw(5) << team.wins
                  << std::right << std::setw(5) << team.draws
                  << std::right << std::setw(5) << team.losses
                  << std::right << std::setw(6) << team.goalsFor
                  << std::right << std::setw(6) << team.goalsAgainst
                  << std::right << std::setw(7) << gdStr
                  << std::right << std::setw(6) << team.points << "\n";
    }
    std::cout << "=========================================================================================\n\n";
    std::cout << " 0. Повернутися в меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;
}

}
