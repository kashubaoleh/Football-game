// Screens.h - екрани гри. Кожен екран отримує турнір і викликає його дії.
#pragma once
#include "../tournament/Tournament.h"

namespace ui {

// Жеребкування (DrawScreens.cpp)
void showDrawMenu(Tournament& tournament);            // жеребкування етапу ліги
void processDraw(Tournament& tournament);
void showPlayoffDrawMenu(Tournament& tournament);     // жеребкування 1/16
void showPlayoffDraw18Menu(Tournament& tournament);   // жеребкування 1/8
void showNextTournamentMenu(Tournament& tournament);  // запуск наступного турніру

// Матчі (MatchScreens.cpp)
void showMatchPreviewMenu(Tournament& tournament);
void processMatchSimulation(Tournament& tournament, int speedMode);
void processPlayoffMatchSimulation(Tournament& tournament, int speedMode);

// Календар (CalendarScreen.cpp) і таблиця (TableScreen.cpp)
void showCalendar(Tournament& tournament);
void showLeagueTable(const LeagueTable& leagueTable);

}
