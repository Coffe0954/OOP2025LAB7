#include "npc.h"
#include <cmath>

std::random_device NPC::rd;
std::mt19937 NPC::gen(NPC::rd());
std::uniform_int_distribution<> NPC::dice_dist(1, 6);

double NPC::calculateDistance(const std::shared_ptr<NPC>& other) const {
    auto pos1 = getPosition();
    auto pos2 = other->getPosition();
    return std::sqrt(std::pow(pos2.first - pos1.first, 2) + 
                     std::pow(pos2.second - pos1.second, 2));
}