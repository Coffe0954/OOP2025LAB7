#ifndef OBSERVER_H
#define OBSERVER_H

#include "../npc/npc.h"
#include <memory>

class Observer {
public:
    virtual ~Observer() = default;
    virtual void onKill(const std::shared_ptr<NPC>& killer, 
                       const std::shared_ptr<NPC>& victim) = 0;
};

#endif