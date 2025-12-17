#ifndef FILE_OBSERVER_H
#define FILE_OBSERVER_H

#include "observer.h"
#include <fstream>

class FileObserver : public Observer {
private:
    std::ofstream logFile;

public:
    FileObserver();
    ~FileObserver();
    
    void onKill(const std::shared_ptr<NPC>& killer, 
                const std::shared_ptr<NPC>& victim) override;
};

#endif