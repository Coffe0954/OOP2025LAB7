#ifndef NPC_FACTORY_H
#define NPC_FACTORY_H

#include "../npc/npc.h"
#include <memory>
#include <string>
#include <random>

class NPCFactory {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    
public:
    static std::shared_ptr<NPC> createNPC(const std::string& type, 
                                         const std::string& name, 
                                         int x, int y);
    
    static std::shared_ptr<NPC> createRandomNPC(const std::string& base_name, 
                                               int map_width, 
                                               int map_height);
    
    static std::string getRandomType();
};

#endif