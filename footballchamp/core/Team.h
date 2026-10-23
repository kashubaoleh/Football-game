// Team.h
#pragma once
#include <string>
#include <vector>

enum class Position { Goalkeeper, Defender, Midfielder, Forward };

struct Player {
    std::string name;
    Position position;
};

struct Team {
    std::string name;
    double attack;
    double defense;

    int initialRating;  // Початковий рейтинг (1..36) для 5-го правила сортування таблиці

    std::vector<Player> squad = {};  // стартовий склад (11 гравців)

    double getTotalRating() const { return attack + defense; }
};
