// main.cpp
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include "io/TeamLoader.h"
#include "sim/Random.h"
#include "tools/BatchSimulation.h"
#include "ui/Console.h"
#include "ui/GameEngine.h"

namespace {

// Seed - ціле число від 0 до 4294967295
bool parseSeed(const char* text, std::uint32_t& seed) {
    if (text == nullptr || *text == '\0') return false;
    for (const char* c = text; *c != '\0'; ++c) {
        if (*c < '0' || *c > '9') return false;
    }
    const unsigned long long value = std::strtoull(text, nullptr, 10);
    if (value > 0xFFFFFFFFULL) return false;
    seed = static_cast<std::uint32_t>(value);
    return true;
}

}  // namespace

// Параметри запуску (необов'язкові):
//   --seed N     відновити турнір із цим seed (його видно в головному меню)
//   --batch N    не грати, а прогнати N турнірів і показати статистику
int main(int argc, char* argv[]) {
    ui::initConsole();

    std::uint32_t seed = Random::randomSeed();
    int batchRuns = 0;
    for (int i = 1; i + 1 < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--batch") {
            batchRuns = std::atoi(argv[i + 1]);
        } else if (option == "--seed" && !parseSeed(argv[i + 1], seed)) {
            std::cout << "Некоректний seed «" << argv[i + 1] << "». Потрібне ціле число від 0 до 4294967295.\n";
            return 1;
        }
    }

    const TeamLoadResult loaded = loadTeams();
    if (!loaded.ok) {
        std::cout << "Не вдалося завантажити команди.\n";
        if (!loaded.path.empty()) std::cout << "Файл: " << loaded.path << "\n";
        for (const auto& error : loaded.errors) std::cout << "  " << error << "\n";
        std::cout << "\nНатисніть ENTER для виходу...";
        std::cin.get();
        return 1;
    }

    if (batchRuns > 0) {
        runBatchSimulation(loaded.teams, batchRuns, seed);
        return 0;
    }

    GameEngine engine(loaded.teams, seed);
    engine.run();
    return 0;
}
