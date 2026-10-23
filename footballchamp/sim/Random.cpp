// Random.cpp
#include "Random.h"
#include <cmath>

namespace {

// SplitMix64: перемішує біти, щоб близькі seed і tag давали зовсім різні потоки
std::uint64_t splitMix64(std::uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

}  // namespace

Random::Random(std::uint64_t seedValue) : seed(seedValue) {
    // seed_seq визначений стандартом однозначно, тож початковий стан однаковий у всіх компіляторах
    std::seed_seq sequence{static_cast<std::uint32_t>(seedValue & 0xFFFFFFFFu),
                           static_cast<std::uint32_t>(seedValue >> 32)};
    engine.seed(sequence);
}

std::uint32_t Random::randomSeed() {
    std::random_device device;
    return static_cast<std::uint32_t>(device());
}

std::uint32_t Random::nextSeed(std::uint32_t seed) {
    return static_cast<std::uint32_t>(splitMix64(static_cast<std::uint64_t>(seed) ^ 0x5EEDBA5EULL) >> 32);
}

Random Random::derive(std::uint64_t tag) const {
    return Random(splitMix64(seed ^ splitMix64(tag)));
}

int Random::nextInt(int minValue, int maxValue) {
    const std::uint32_t range = static_cast<std::uint32_t>(maxValue - minValue) + 1u;
    if (range <= 1u) return minValue;
    // Відкидаємо "хвіст", щоб розподіл був рівномірним без зсуву
    const std::uint32_t threshold = (0u - range) % range;
    std::uint32_t x = nextBits();
    while (x < threshold) x = nextBits();
    return minValue + static_cast<int>(x % range);
}

double Random::nextReal(double minValue, double maxValue) {
    const std::uint64_t high = nextBits() >> 5;  // 27 біт
    const std::uint64_t low = nextBits() >> 6;   // 26 біт
    const double unit = (static_cast<double>(high) * 67108864.0 + static_cast<double>(low)) / 9007199254740992.0;
    return minValue + (maxValue - minValue) * unit;
}

bool Random::chance(double probability) {
    return nextReal(0.0, 1.0) < probability;
}

int Random::pickWeighted(const std::vector<double>& weights) {
    double total = 0.0;
    for (double weight : weights) {
        if (weight > 0.0) total += weight;
    }
    if (total <= 0.0) return -1;

    double point = nextReal(0.0, total);
    int last = -1;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        if (weights[i] <= 0.0) continue;  // гравці з нульовою вагою не обираються ніколи
        last = static_cast<int>(i);
        if (point < weights[i]) return last;
        point -= weights[i];
    }
    return last;  // лише через похибку округлення
}

int Random::poissonSmall(double mean) {
    const double limit = std::exp(-mean);
    double product = 1.0;
    int k = 0;
    do {
        ++k;
        product *= nextReal(0.0, 1.0);
    } while (product > limit);
    return k - 1;
}

int Random::poisson(double mean) {
    if (mean <= 0.0) return 0;
    int total = 0;
    while (mean > 30.0) {  // сума незалежних Пуассонів теж Пуассон
        total += poissonSmall(30.0);
        mean -= 30.0;
    }
    return total + poissonSmall(mean);
}
