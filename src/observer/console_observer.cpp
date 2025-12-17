#include "console_observer.h"
#include <iostream>
#include <mutex>

//глобальный мьютекс для защиты std::cout
static std::mutex cout_mutex;

void ConsoleObserver::onKill(const std::shared_ptr<NPC>& killer, 
                            const std::shared_ptr<NPC>& victim) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << "[KILL] " << killer->getType() << " " << killer->getName()
              << " killed " << victim->getType() << " " << victim->getName() 
              << std::endl;
}