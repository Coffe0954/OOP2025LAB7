#ifndef CONSOLE_OBSERVER_H
#define CONSOLE_OBSERVER_H

#include "observer.h"

class ConsoleObserver : public Observer {
public:
    void onKill(const std::shared_ptr<NPC>& killer, 
                const std::shared_ptr<NPC>& victim) override;
};

#endif