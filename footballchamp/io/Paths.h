// Paths.h - пошук файлів даних (уся платформо-залежна частина тут)
#pragma once
#include <string>
#include <vector>

std::string executableDirectory();                                        // тека програми з кінцевим '/', або ""
std::vector<std::string> dataFileCandidates(const std::string& fileName); // де шукати файл (за порядком)
std::string findDataFile(const std::string& fileName);                    // перший існуючий шлях або ""
