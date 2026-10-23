// Broadcast.h - текстова трансл€ц≥€ матчу
#pragma once
#include "../core/Match.h"

namespace ui {

int delayForSpeed(int speedMode);                        // затримка кадру за режимом швидкост≥ (2-4)
void playMatchBroadcast(const Match& match, int delayMs); // програЇ вже з≥граний матч хвилина за хвилиною

}
