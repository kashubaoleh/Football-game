// Random.h - єдиний генератор випадкових чисел гри. Відтворюється за seed.
//
// Розподіли (ціле, дійсне, Пуассон, перемішування) написані тут, а не взяті зі <random>:
// у різних компіляторах std::poisson_distribution, std::uniform_int_distribution
// та std::shuffle з однаковим seed дають різні числа, а нам потрібно, щоб турнір
// за seed відновлювався однаково. Сам std::mt19937 стандартом визначений однозначно.
#pragma once
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

class Random {
public:
    explicit Random(std::uint64_t seed);

    // Випадковий seed для нового турніру. Єдине місце, де використовується random_device.
    static std::uint32_t randomSeed();

    // Наступний seed у ланцюжку: з нього починається наступний турнір сесії.
    static std::uint32_t nextSeed(std::uint32_t seed);

    // Незалежний генератор для окремого завдання (жеребкування, матч).
    // Однаковий tag завжди дає однаковий потік чисел, різні tag - різні.
    Random derive(std::uint64_t tag) const;

    int nextInt(int minValue, int maxValue);            // ціле з [min, max]
    double nextReal(double minValue, double maxValue);  // дійсне з [min, max)
    bool chance(double probability);                    // true з імовірністю probability
    int poisson(double mean);                           // розподіл Пуассона

    // Індекс за вагами (чим більша вага, тим імовірніший). -1, якщо всі ваги <= 0.
    int pickWeighted(const std::vector<double>& weights);

    template <typename T>
    void shuffle(std::vector<T>& items) {               // Фішер-Йейтс
        for (std::size_t i = items.size(); i > 1; --i) {
            const std::size_t j = static_cast<std::size_t>(nextInt(0, static_cast<int>(i) - 1));
            std::swap(items[i - 1], items[j]);
        }
    }

private:
    std::uint64_t seed;
    std::mt19937 engine;

    std::uint32_t nextBits() { return static_cast<std::uint32_t>(engine()); }
    int poissonSmall(double mean);                      // алгоритм Кнута, для невеликих mean
};
