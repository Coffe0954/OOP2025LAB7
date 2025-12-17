#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "../npc/npc.h"
#include "../observer/observer.h"
#include "battle_queue.h"
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <condition_variable>
#include <map>
#include <set>
#include <chrono>

class GameManager {
private:
    std::vector<std::shared_ptr<NPC>> npcs;
    mutable std::shared_mutex npcs_mutex;
    
    BattleQueue battle_queue;
    
    std::vector<std::shared_ptr<Observer>> observers;
    
    std::atomic<bool> game_running{false};
    std::atomic<int> game_time{0};
    std::atomic<int> total_battles{0};
    std::atomic<int> total_kills{0};
    
    std::thread movement_thread;
    std::thread battle_thread;
    std::thread display_thread;
    
    static constexpr int MAP_WIDTH = 100;
    static constexpr int MAP_HEIGHT = 100;
    static constexpr int TOTAL_NPCS = 50;
    static constexpr int GAME_DURATION = 30;
    
    //мьютекс для защиты std::cout
    static std::mutex cout_mutex;
    
public:
    GameManager();
    ~GameManager();
    
    //основные методы
    void initialize();
    void start();
    void stop();
    void run();
    
    void printStatistics() const;
    void printFinalReport() const;
    
private:
    //потоки
    void movementWorker();
    void battleWorker();
    void displayWorker();
    
    //утилиты
    void printMap() const;
    void addRandomNPCs(int count);
    bool checkCollision(const std::shared_ptr<NPC>& a, 
                       const std::shared_ptr<NPC>& b) const;
    void processBattle(const BattleTask& task);
    void resolveBattle(std::shared_ptr<NPC> attacker, 
                      std::shared_ptr<NPC> defender);
    
    void safePrint(const std::string& message) const;
    int countAliveNPCs() const;
    std::map<std::string, int> countNPCsByType() const;
};

#endif