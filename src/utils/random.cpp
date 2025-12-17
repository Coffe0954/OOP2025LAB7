#include "random.h"

std::random_device Random::rd;
std::mt19937 Random::gen(Random::rd());

int Random::getInt(int min, int max) {
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

double Random::getDouble(double min, double max) {
    std::uniform_real_distribution<> dist(min, max);
    return dist(gen);
}

bool Random::getBool(double probability) {
    std::bernoulli_distribution dist(probability);
    return dist(gen);
}