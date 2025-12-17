#ifndef RANDOM_H
#define RANDOM_H

#include <random>

class Random {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    
public:
    static int getInt(int min, int max);
    static double getDouble(double min, double max);
    static bool getBool(double probability = 0.5);
};

#endif