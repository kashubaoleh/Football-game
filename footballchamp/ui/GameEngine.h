// GameEngine.h - головне меню гри. ”с€ лог≥ка турн≥ру живе в Tournament,
// екрани в ui/*Screens.cpp, трансл€ц≥€ в ui/Broadcast.cpp.
#pragma once
#include <cstdint>
#include <vector>
#include "../core/Team.h"
#include "../tournament/Tournament.h"

class GameEngine {
public:
    GameEngine(const std::vector<Team>& teams, std::uint32_t seed);
    void run();

private:
    Tournament tournament;
};
