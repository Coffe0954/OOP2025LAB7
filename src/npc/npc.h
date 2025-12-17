#ifndef NPC_H
#define NPC_H

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <random>
#include <vector>
#include <cmath>

class Observer;

class NPC {
public:
    virtual ~NPC() = default;
    
    virtual void print() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    virtual std::pair<int, int> getPosition() const = 0;
    
    virtual bool isAlive() const = 0;
    virtual void setAlive(bool alive) = 0;
    virtual void moveRandomly(int map_width, int map_height) = 0;
    virtual bool canKill(const std::shared_ptr<NPC>& other) const = 0;
    
    virtual int rollAttackDice() = 0;
    virtual int rollDefenseDice() = 0;
    
    virtual int getMoveDistance() const = 0;
    virtual int getKillDistance() const = 0;

    virtual void addObserver(const std::shared_ptr<Observer>& observer) = 0;
    virtual void notifyKill(const std::shared_ptr<NPC>& victim) = 0;

    virtual void save(std::ostream& file) const = 0;
    
    virtual double calculateDistance(const std::shared_ptr<NPC>& other) const;
    
protected:
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_int_distribution<> dice_dist;
};

#endif