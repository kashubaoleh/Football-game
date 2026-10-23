// Printers.h - спільні функції виводу: шапки, рядки матчів, корзини, прев'ю жеребкування
#pragma once
#include <string>
#include <vector>
#include "../core/Match.h"
#include "../core/Team.h"
#include "../tournament/PlayoffManager.h"

namespace ui {

void printMatchHeader(const Match& match);
std::string describeGoal(const GoalEvent& ev);                // "Мбаппе (ПСЖ)" або лише команда, якщо автор невідомий
void printQuickResult(const Match& match);                    // рахунок і список голів
void printMatchRow(size_t number, const Match& m, const std::string& middle, bool showStatus);
std::string leagueScoreMiddle(const Match& m);                // "  2 - 1  " або "    vs    "
std::string playoffScoreMiddle(const Match& m);               // "  [2 - 1] (4 - 3)  " або "    vs    "
void printFirstLegPairs(const PlayoffRound& round);           // пари перших матчів раунду
void printDrawPreview(const std::vector<DrawPreviewPair>& pairs);
void printPots(const std::vector<std::vector<Team>>& pots);

}
