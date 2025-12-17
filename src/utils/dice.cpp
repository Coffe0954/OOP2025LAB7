#include "dice.h"

std::random_device Dice::rd;
std::mt19937 Dice::gen(Dice::rd());

int Dice::roll(int sides) {
    if (sides <= 0) return 0;
    std::uniform_int_distribution<> dist(1, sides);
    return dist(gen);
}

int Dice::rollRange(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

int Dice::rollMultiple(int count, int sides) {
    int total = 0;
    for (int i = 0; i < count; ++i) {
        total += roll(sides);
    }
    return total;
}

int Dice::rollWithModifier(int sides, int modifier) {
    return roll(sides) + modifier;
}