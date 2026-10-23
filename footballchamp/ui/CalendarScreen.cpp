// CalendarScreen.cpp - календар матчів і швидка промотка турніру
#include "Screens.h"
#include <climits>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "Console.h"
#include "Printers.h"

namespace ui {

namespace {

// Розбір невід'ємного цілого з рядка. Дуже великі числа обрізаються до INT_MAX.
// false, якщо рядок порожній або містить щось, крім цифр.
bool parseNonNegativeInt(const std::string& text, int& value) {
    if (text.empty()) return false;
    long long result = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') return false;
        result = result * 10 + (ch - '0');
        if (result > INT_MAX) result = INT_MAX;
    }
    value = static_cast<int>(result);
    return true;
}

// Питає кількість матчів для промотки. Повертає число > 0 або 0, якщо гравець скасував.
int askMatchCount() {
    std::cout << "\nСкільки матчів просимулювати? (ціле число більше 0, 0 - скасувати): ";
    std::string token;
    int value = 0;
    while (true) {
        if (!(std::cin >> token)) {
            std::cin.clear();
            return 0;
        }
        if (parseNonNegativeInt(token, value)) return value;
        std::cout << "Некоректне введення. Введіть ціле число більше 0 (0 - скасувати): ";
    }
}

// Пункт меню календаря
struct CalendarAction {
    std::string label;
    bool askCount;        // true: спитати кількість матчів; false: промотка до віхи
    Milestone milestone;
};

struct MilestoneAction {
    Milestone milestone;
    const char* label;
};

// Пункт показується, лише поки турнір ще не дійшов до цієї віхи
const MilestoneAction kMilestoneActions[] = {
    {Milestone::LeagueEnd,           "Просимулювати до початку 1/16"},
    {Milestone::RoundOf8Start,       "Просимулювати до початку 1/8"},
    {Milestone::QuarterFinalsStart,  "Просимулювати до початку 1/4"},
    {Milestone::SemiFinalsStart,     "Просимулювати до початку 1/2"},
    {Milestone::FinalStart,          "Просимулювати до початку Фіналу"},
    {Milestone::TournamentEnd,       "Просимулювати Турнір до кінця"},
};

void printCalendar(const Tournament& tournament) {
    clearScreen();
    std::cout << "=======================================================\n";
    std::cout << "                  КАЛЕНДАР МАТЧІВ                      \n";
    std::cout << "=======================================================\n\n";

    if (!tournament.isDrawCompleted()) {
        std::cout << " Жеребкування ще не проведено!\n\n";
    } else {
        // Вивід Етапу Ліги
        for (const auto& round : tournament.getSchedule()) {
            std::cout << "=== ЕТАП ЛІГИ | ТУР № " << round.roundNumber << " ===\n";
            for (size_t i = 0; i < round.matches.size(); ++i) {
                const auto& m = round.matches[i];
                printMatchRow(i + 1, m, leagueScoreMiddle(m), true);
            }
            std::cout << "\n";
        }

        // Вивід Плей-оф (якщо він почався)
        if (tournament.isPlayoffStarted()) {
            std::cout << "=======================================================\n";
            std::cout << "                    ПЛЕЙ-ОФ                            \n";
            std::cout << "=======================================================\n\n";

            for (const auto& r : tournament.getPlayoff().getAllRounds()) {
                if (r.matches.empty()) continue;
                std::cout << "=== " << r.roundName << " ===\n";


                for (size_t i = 0; i < r.matches.size(); ++i) {
                    const auto& m = r.matches[i];
                    printMatchRow(i + 1, m, playoffScoreMiddle(m), true);
                }
                std::cout << "\n";
            }
        }
    }

}

} // namespace

void showCalendar(Tournament& tournament) {
    while (true) {
        printCalendar(tournament);

        // Пункти швидкої промотки залежать від того, на якій стадії зараз турнір
        std::vector<CalendarAction> actions;
        if (!tournament.isTournamentFinished()) {
            actions.push_back({"Просимулювати деяку кількість матчів", true, Milestone::TournamentEnd});
            for (const auto& item : kMilestoneActions) {
                if (!tournament.isMilestoneReached(item.milestone)) {
                    actions.push_back({item.label, false, item.milestone});
                }
            }
        }

        std::cout << "=======================================================\n";
        for (size_t i = 0; i < actions.size(); ++i) {
            std::cout << " " << i + 1 << ". " << actions[i].label << "\n";
        }
        std::cout << " 0. Повернутися в меню\n";
        std::cout << "\nВибір: ";

        size_t choice = 0;
        while (true) {
            std::string token;
            int value = 0;
            if (!(std::cin >> token)) {
                std::cin.clear();
                return;
            }
            if (parseNonNegativeInt(token, value) && static_cast<size_t>(value) <= actions.size()) {
                choice = static_cast<size_t>(value);
                break;
            }
            std::cout << "Невірний вибір. Спробуйте ще раз: ";
        }

        if (choice == 0) return;

        const CalendarAction& action = actions[choice - 1];
        if (action.askCount) {
            int count = askMatchCount();
            if (count > 0) tournament.simulateMatches(count);
        } else {
            tournament.simulateUntil(action.milestone);
        }
        // Далі цикл знову очищає консоль і показує календар із промотаними матчами
    }
}

}
