#ifndef BEAR_H
#define BEAR_H

#include "npc.h"
#include "../observer/observer.h"
#include <memory>
#include <vector>
#include <mutex>

class Bear : public NPC, public std::enable_shared_from_this<Bear> {
private:
    std::string name;
    int x, y;
    std::atomic<bool> alive;
    mutable std::mutex mtx;
    std::vector<std::shared_ptr<Observer>> observers;
    
public:
    Bear(const std::string& name, int x, int y);
    
    void print() const override;
    std::string getName() const override;
    std::string getType() const override;
    std::pair<int, int> getPosition() const override;
    
    bool isAlive() const override;
    void setAlive(bool alive) override;
    void moveRandomly(int map_width, int map_height) override;
    bool canKill(const std::shared_ptr<NPC>& other) const override;
    
    int rollAttackDice() override;
    int rollDefenseDice() override;
    
    int getMoveDistance() const override { return 5; }
    int getKillDistance() const override { return 10; }
    
    void addObserver(const std::shared_ptr<Observer>& observer) override;
    void notifyKill(const std::shared_ptr<NPC>& victim) override;
    
    void save(std::ostream& file) const override;
};

#endif