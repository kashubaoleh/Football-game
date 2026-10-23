// Printers.cpp
#include "Printers.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace ui {

namespace {

// "А / Б" - команди, що претендують на одне місце в парі
std::string joinCandidates(const std::vector<Team>& teams) {
    std::string s;
    for (size_t i = 0; i < teams.size(); ++i) {
        if (i > 0) s += " / ";
        s += teams[i].name;
    }
    return s;
}

} // namespace

std::string describeGoal(const GoalEvent& ev) {
    if (ev.scorerName.empty()) return ev.teamName;
    return ev.scorerName + " (" + ev.teamName + ")";
}

void printMatchHeader(const Match& match) {
    std::cout << "=======================================================\n";
    std::cout << "             " << match.homeTeam.name << "  vs  " << match.awayTeam.name << "\n";
    std::cout << "=======================================================\n\n";
}

// Швидкий показ результату: рахунок і список голів
void printQuickResult(const Match& match) {
    std::cout << "МАТЧ ЗАВЕРШЕНО!\n";
    std::cout << "РАХУНОК: " << match.resultLog.homeGoals << " - " << match.resultLog.awayGoals << "\n\n";
    std::cout << "ПОДІЇ МАТЧУ:\n";
    if (match.resultLog.events.empty()) {
        std::cout << " (Без забитих голів)\n";
    } else {
        for (const auto& ev : match.resultLog.events) {
            std::cout << "  " << ev.displayMinute << " -  ГОЛ! " << describeGoal(ev) << " [Рахунок: "
                      << ev.matchHomeScore << ":" << ev.matchAwayScore << "]\n";
        }
    }
}

// Рядок матчу: "<№>. <господарі><середина><гості>[ статус]"
void printMatchRow(size_t number, const Match& m, const std::string& middle, bool showStatus) {
    std::cout << std::right << std::setw(2) << number << ". "
              << std::setw(36) << std::right << m.homeTeam.name
              << middle
              << std::setw(36) << std::left << m.awayTeam.name;
    if (showStatus) {
        std::cout << (m.isPlayed ? " [ЗІГРАНО]" : " [ЗАПЛАНОВАНО]");
    }
    std::cout << "\n";
}

// Середина рядка для матчу ліги: "  2 - 1  " або "    vs    "
std::string leagueScoreMiddle(const Match& m) {
    if (!m.isPlayed) return "    vs    ";
    return "  " + std::to_string(m.resultLog.homeGoals) + " - " + std::to_string(m.resultLog.awayGoals) + "  ";
}

// Середина рядка для матчу плей-оф: "  [2 - 1] (4 - 3)  " (пенальті, якщо були) або "    vs    "
std::string playoffScoreMiddle(const Match& m) {
    if (!m.isPlayed) return "    vs    ";
    std::string s = "  [" + std::to_string(m.resultLog.homeGoals) + " - " + std::to_string(m.resultLog.awayGoals) + "]";
    if (m.resultLog.homePenaltyGoals != -1) {
        s += " (" + std::to_string(m.resultLog.homePenaltyGoals) + " - "
                  + std::to_string(m.resultLog.awayPenaltyGoals) + ")";
    }
    return s + "  ";
}

// Перші матчі дуелей раунду (перша половина списку матчів)
void printFirstLegPairs(const PlayoffRound& round) {
    size_t halfSize = round.matches.size() / 2;
    for (size_t i = 0; i < halfSize; ++i) {
        const auto& m = round.matches[i];
        std::cout << i + 1 << ". " << m.homeTeam.name << " vs " << m.awayTeam.name << "\n";
    }
}

// Пари ще не згенерованої сітки (у порядку, в якому будуть перші матчі)
void printDrawPreview(const std::vector<DrawPreviewPair>& pairs) {
    for (size_t i = 0; i < pairs.size(); ++i) {
        std::cout << std::right << std::setw(2) << i + 1 << ". "
                  << std::setw(38) << std::right << joinCandidates(pairs[i].home)
                  << "  vs  "
                  << std::setw(38) << std::left << joinCandidates(pairs[i].away) << "\n";
    }
}

// Корзини поруч одна з одною; порядок команд - як у жеребкуванні (за рейтингом)
void printPots(const std::vector<std::vector<Team>>& pots) {
    const int columnWidth = 24;
    size_t rows = 0;

    for (size_t p = 0; p < pots.size(); ++p) {
        std::cout << std::left << std::setw(columnWidth) << ("КОРЗИНА " + std::to_string(p + 1));
        rows = std::max(rows, pots[p].size());
    }
    std::cout << "\n";
    for (size_t p = 0; p < pots.size(); ++p) {
        std::cout << std::left << std::setw(columnWidth) << std::string(columnWidth - 3, '-');
    }
    std::cout << "\n";

    for (size_t r = 0; r < rows; ++r) {
        for (const auto& pot : pots) {
            std::string cell;
            if (r < pot.size()) {
                std::string number = std::to_string(pot[r].initialRating);
                if (number.size() < 2) number = " " + number;
                cell = number + ". " + pot[r].name;
            }
            std::cout << std::left << std::setw(columnWidth) << cell;
        }
        std::cout << "\n";
    }
}

}
