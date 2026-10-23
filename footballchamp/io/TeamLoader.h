// TeamLoader.h - завантаження команд і їхніх стартових складів із текстового файлу (data/teams.txt)
#pragma once
#include <string>
#include <vector>
#include "../core/Team.h"

struct TeamLoadResult {
    bool ok = false;
    std::vector<Team> teams;
    std::vector<std::string> errors;  // зрозумілі гравцю повідомлення (усі одразу)
    std::string path;                 // файл, який читали
};

// Читає команди з указаного файлу (UTF-8 з BOM або без, або Windows-1251)
TeamLoadResult loadTeamsFromFile(const std::string& path);

// Шукає teams.txt у типових місцях і читає його
TeamLoadResult loadTeams();
