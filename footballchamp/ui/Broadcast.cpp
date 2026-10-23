// Broadcast.cpp
#include "Broadcast.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "../core/Config.h"
#include "Console.h"
#include "Printers.h"

namespace ui {

// Затримка кадру трансляції за режимом швидкості (2 - швидка, 3 - середня, 4 - повільна)
int delayForSpeed(int speedMode) {
    if (speedMode == 2) return Config::DELAY_FAST;
    if (speedMode == 3) return Config::DELAY_MEDIUM;
    if (speedMode == 4) return Config::DELAY_SLOW;
    return 100;
}

void playMatchBroadcast(const Match& match, int delayMs) {
    const MatchLog& log = match.resultLog;

    size_t eventIndex = 0;
    int currentHomeGoals = 0, currentAwayGoals = 0;
    int homePens = 0, awayPens = 0;
    bool showPens = false;
    std::vector<std::string> matchLogLines;

    auto printHeader = [&](const std::string& minuteText) {
        std::cout << "=======================================================\n";
        std::cout << "                  ТРАНСЛЯЦІЯ МАТЧУ                     \n";
        std::cout << "=======================================================\n";
        std::cout << " [ " << std::setw(6) << std::left << minuteText << " ]   "
                  << match.homeTeam.name << " " << currentHomeGoals
                  << " - " << currentAwayGoals << " " << match.awayTeam.name;
        if (showPens) std::cout << "  (пен. " << homePens << " - " << awayPens << ")";
        std::cout << "\n-------------------------------------------------------\n";
        std::cout << "ПОДІЇ МАТЧУ:\n";
    };

    auto renderFrame = [&](const std::string& minuteText, bool longPause) {
        clearScreen();
        printHeader(minuteText);
        if (matchLogLines.empty()) {
            std::cout << " (Іде спокійна гра...)\n";
        } else {
            for (const auto& line : matchLogLines) std::cout << line << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(longPause ? delayMs * 4 : delayMs));
    };

    // Додає в журнал усі голи з ключем <= key; true, якщо був гол
    auto flushEvents = [&](int key) -> bool {
        bool goal = false;
        while (eventIndex < log.events.size() &&
               goalEventSortKey(log.events[eventIndex]) <= key) {
            const auto& ev = log.events[eventIndex];
            currentHomeGoals = ev.matchHomeScore;
            currentAwayGoals = ev.matchAwayScore;
            matchLogLines.push_back("  " + ev.displayMinute + " - ГОЛ! " + describeGoal(ev) +
                                    " [Рахунок: " + std::to_string(currentHomeGoals) +
                                    ":" + std::to_string(currentAwayGoals) + "]");
            goal = true;
            ++eventIndex;
        }
        return goal;
    };

    // Один тайм: хвилини from..to, потім доданий час
    auto playPeriod = [&](int from, int to, int stoppage) {
        for (int m = from; m <= to; ++m) {
            bool g = flushEvents(m * 100);
            renderFrame(std::to_string(m) + "'", g);
        }
        for (int e = 1; e <= stoppage; ++e) {
            bool g = flushEvents(to * 100 + e);
            renderFrame(std::to_string(to) + "+" + std::to_string(e) + "'", g);
        }
    };

    auto addBreak = [&](const std::string& text, const std::string& label) {
        matchLogLines.push_back(text);
        renderFrame(label, true);
    };

    // --- Основний час ---
    playPeriod(1, 45, log.stoppage[0]);
    addBreak("  45' --- ПЕРЕРВА ---", "45'");
    playPeriod(46, 90, log.stoppage[1]);

    std::string lastLabel = "90'";

    // --- Додатковий час ---
    if (log.extraTimePlayed) {
        addBreak("  90' --- КІНЕЦЬ ОСНОВНОГО ЧАСУ. ДОДАТКОВИЙ ЧАС! ---", "90'");
        playPeriod(91, 105, log.stoppage[2]);
        addBreak("  105' --- ПЕРЕРВА В ДОДАТКОВОМУ ЧАСІ ---", "105'");
        playPeriod(106, 120, log.stoppage[3]);
        lastLabel = "120'";
        matchLogLines.push_back("  120' --- КІНЕЦЬ ДОДАТКОВОГО ЧАСУ ---");
    }

    // --- Пенальті ---
    if (!log.penaltyShootout.empty()) {
        matchLogLines.push_back("  --- СЕРІЯ ПЕНАЛЬТІ ---");
        showPens = true;
        renderFrame("PEN", true);

        for (const auto& p : log.penaltyShootout) {
            if (p.isScored) {
                if (p.teamName == match.homeTeam.name) ++homePens; else ++awayPens;
            }
            matchLogLines.push_back("  PEN " + p.teamName + " (удар " + std::to_string(p.attemptNumber) +
                                    ") - " + (p.isScored ? "ГОЛ" : "ПРОМАХ") +
                                    "  [" + std::to_string(homePens) + ":" + std::to_string(awayPens) + "]");
            renderFrame("PEN", true);
        }
    } else {
        matchLogLines.push_back("  " + lastLabel + " --- ФІНАЛЬНИЙ СВИСТОК! ---");
    }

    // --- Підсумковий екран ---
    clearScreen();
    printHeader(lastLabel);
    for (const auto& line : matchLogLines) std::cout << line << "\n";
    std::cout << "=======================================================\n";
    std::cout << " МАТЧ ЗАВЕРШЕНО! Підсумковий рахунок: "
              << log.homeGoals << " - " << log.awayGoals;
    if (log.extraTimePlayed) std::cout << " (з урахуванням додаткового часу)";
    std::cout << "\n";
    if (log.homePenaltyGoals != -1) {
        std::cout << " Серія пенальті: " << log.homePenaltyGoals << " - " << log.awayPenaltyGoals << "\n";
    }
    std::cout << "=======================================================\n";
}

}
