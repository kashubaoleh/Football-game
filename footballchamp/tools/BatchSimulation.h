// BatchSimulation.h - масова симул€ц≥€ турн≥р≥в дл€ перев≥рки балансу
#pragma once
#include <cstdint>
#include <vector>
#include "../core/Team.h"

// seed визначаЇ всю сер≥ю: з тим самим seed вийде та сама статистика
void runBatchSimulation(const std::vector<Team>& teams, int runs, std::uint32_t seed);
