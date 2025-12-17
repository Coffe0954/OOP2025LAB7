#ifndef DICE_H
#define DICE_H

#include <random>

class Dice {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    
public:
    // бросить кубик с указанным количеством граней
    static int roll(int sides = 6);
    
    // бросить кубик с заданным диапазоном
    static int rollRange(int min, int max);
    
    // бросить несколько кубиков и получить сумму
    static int rollMultiple(int count, int sides = 6);
    
    // бросить кубик с модификатором
    static int rollWithModifier(int sides, int modifier);
};

#endif