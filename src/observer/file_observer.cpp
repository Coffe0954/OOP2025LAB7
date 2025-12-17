#include "file_observer.h"
#include <mutex>

static std::mutex file_mutex;

FileObserver::FileObserver() {
    logFile.open("log.txt", std::ios::app);
}

FileObserver::~FileObserver() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void FileObserver::onKill(const std::shared_ptr<NPC>& killer, 
                         const std::shared_ptr<NPC>& victim) {
    std::lock_guard<std::mutex> lock(file_mutex);
    if (logFile.is_open()) {
        logFile << "[KILL] " << killer->getType() << " " << killer->getName()
                << " killed " << victim->getType() << " " << victim->getName() 
                << std::endl;
    }
}