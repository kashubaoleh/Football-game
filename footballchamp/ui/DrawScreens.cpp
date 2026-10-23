// DrawScreens.cpp - екрани жеребкувань
#include "Screens.h"
#include <iomanip>
#include <iostream>
#include "../tournament/LeagueDraw.h"
#include "Console.h"
#include "Printers.h"

namespace ui {

void showDrawMenu(Tournament& tournament) {
    clearScreen();
    std::cout << "=======================================================\n";
    std::cout << "               ЖЕРЕБКУВАННЯ ЕТАПУ ЛІГИ                 \n";
    std::cout << "=======================================================\n\n";
    LeagueDraw preview(tournament.getTeams());
    std::cout << "Команди в корзинах (у порядку жеребкування):\n\n";
    printPots(preview.getPots());
    std::cout << "\nБажаєте розпочати жеребкування календаря для 36 команд?\n";
    std::cout << " 1. Підтвердити та згенерувати розклад\n";
    std::cout << " 2. Повернутися в меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;

    if (choice == 1) {
        processDraw(tournament);
    }
}

void processDraw(Tournament& tournament) {
    tournament.performLeagueDraw();
    clearScreen();
    std::cout << "Жеребкування успішно проведено! Згенеровано 8 турів за схемою.\n\n";

    for (const auto& round : tournament.getSchedule()) {
        std::cout << "--- ТУР № " << round.roundNumber << " ---\n";
        for (size_t i = 0; i < round.matches.size(); ++i) {
            printMatchRow(i + 1, round.matches[i], "  vs  ", false);
        }
        std::cout << "\n";
    }

    std::cout << "=======================================================\n";
    std::cout << " 1. Перейти до першого матчу\n";
    std::cout << " 2. Перейти до меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;

    if (choice == 1) {
        showMatchPreviewMenu(tournament);
    }
}

void showPlayoffDrawMenu(Tournament& tournament) {
    clearScreen();
    std::cout << "=======================================================\n";
    std::cout << "              ЖЕРЕБКУВАННЯ 1/16 ФІНАЛУ                \n";
    std::cout << "=======================================================\n\n";
    std::cout << "Етап ліги завершено! Команди з 9 по 24 місце зіграють у стикових матчах.\n\n";
    std::cout << "Сітка 1/16 фіналу ще не згенерована. Можливі пари перших матчів\n";
    std::cout << "(ліворуч - господарі; команди через / претендують на одне місце в парі):\n\n";
    printDrawPreview(tournament.getPlayoff().previewRoundOf16(tournament.getLeagueTable().getTable(), tournament.getTeams()));
    std::cout << "\nПари 1-4 і 5-8 мають тих самих претендентів: з двох команд кожного боку\n";
    std::cout << "одна потрапляє в пару 1-4, інша - в пару 5-8.\n\n";
    std::cout << " 1. Провести жеребкування та сформувати сітку 1/16\n";
    std::cout << " 2. Повернутися в меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;

    if (choice == 1) {
        // Генеруємо 1/16 (перші матчі та матчі-відповіді створюються автоматично)
        tournament.performRoundOf16Draw();

        clearScreen();
        std::cout << "Жеребкування 1/16 успішно проведено!\n\n=== ПАРИ 1/16 ФІНАЛУ (Перші матчі) ===\n";
        // Виводимо тільки перші 8 матчів для наочності жеребкування
        printFirstLegPairs(tournament.getPlayoff().getCurrentRound());
        waitEnter();
    }
}

void showPlayoffDraw18Menu(Tournament& tournament) {
    clearScreen();
    std::cout << "=======================================================\n";
    std::cout << "              ЖЕРЕБКУВАННЯ 1/8 ФІНАЛУ                 \n";
    std::cout << "=======================================================\n\n";
    std::cout << "Переможці 1/16 та топ-8 команд ліги чекають на жеребкування.\n\n";
    std::cout << "Сітка 1/8 фіналу ще не згенерована. Можливі пари перших матчів\n";
    std::cout << "(ліворуч - переможці 1/16, господарі; праворуч - команди з топ-8 ліги,\n";
    std::cout << " через / вказано претендентів на одне місце в парі):\n\n";
    printDrawPreview(tournament.getPlayoff().previewRoundOf8(tournament.getLeagueTopTeams(8)));
    std::cout << "\nПари 1-4 і 5-8 з однаковими претендентами праворуч ділять їх між собою:\n";
    std::cout << "одна команда дістається парі 1-4, інша - парі 5-8.\n\n";
    std::cout << " 1. Згенерувати пари 1/8 фіналу\n";
    std::cout << " 2. Повернутися в меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;

    if (choice == 1) {
        // Генеруємо 1/8 (усередині методу вже створюються і перші матчі, і відповіді)
        tournament.performRoundOf8Draw();

        clearScreen();
        std::cout << "Жеребкування 1/8 успішно проведено!\n\n=== ПАРИ 1/8 ФІНАЛУ (Перші матчі) ===\n";
        printFirstLegPairs(tournament.getPlayoff().getCurrentRound());
        waitEnter();
    }
}

void showNextTournamentMenu(Tournament& tournament) {
    clearScreen();
    std::cout << "=======================================================\n";
    std::cout << "                    НОВИЙ ТУРНІР                       \n";
    std::cout << "=======================================================\n\n";
    std::cout << "Турнір завершено. Запустити наступний?\n";
    std::cout << "Усі дані поточного турніру (матчі, таблицю, плей-оф) буде стерто.\n";
    std::cout << " 1. Запустити наступний турнір\n";
    std::cout << " 2. Повернутися в меню\n";
    std::cout << "\nВибір: ";

    int choice = 0;
    std::cin >> choice;

    if (choice == 1) {
        tournament.reset();
    }
}

}
