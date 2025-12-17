#include "npc_factory.h"
#include "../npc/bear.h"
#include "../npc/werewolf.h"
#include "../npc/bandit.h"
#include <stdexcept>

std::random_device NPCFactory::rd;
std::mt19937 NPCFactory::gen(NPCFactory::rd());

std::shared_ptr<NPC> NPCFactory::createNPC(const std::string& type, 
                                          const std::string& name, 
                                          int x, int y) {
    if (x < 0 || y < 0) {
        throw std::invalid_argument("Coordinates must be non-negative");
    }
    
    if (type == "Bear") {
        return std::make_shared<Bear>(name, x, y);
    } else if (type == "Werewolf") {
        return std::make_shared<Werewolf>(name, x, y);
    } else if (type == "Bandit") {
        return std::make_shared<Bandit>(name, x, y);
    }
    
    throw std::invalid_argument("Unknown NPC type: " + type);
}

std::shared_ptr<NPC> NPCFactory::createRandomNPC(const std::string& base_name, 
                                                int map_width, 
                                                int map_height) {
    std::string type = getRandomType();
    std::string name = type + "_" + base_name;
    
    std::uniform_int_distribution<> x_dist(0, map_width - 1);
    std::uniform_int_distribution<> y_dist(0, map_height - 1);
    
    int x = x_dist(gen);
    int y = y_dist(gen);
    
    return createNPC(type, name, x, y);
}

std::string NPCFactory::getRandomType() {
    std::uniform_int_distribution<> type_dist(0, 2);
    int type_num = type_dist(gen);
    
    switch (type_num) {
        case 0: return "Bear";
        case 1: return "Werewolf";
        case 2: return "Bandit";
        default: return "Bear";
    }
}