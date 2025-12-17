#include "bear.h"
#include <iostream>
#include <random>
#include <fstream>

Bear::Bear(const std::string& name, int x, int y) 
    : name(name), x(x), y(y), alive(true) {}

void Bear::print() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Bear " << name << " at (" << x << ", " << y << ")" 
              << (alive ? " [ALIVE]" : " [DEAD]") << std::endl;
}

std::string Bear::getName() const { 
    std::lock_guard<std::mutex> lock(mtx);
    return name; 
}

std::string Bear::getType() const { 
    return "Bear"; 
}

std::pair<int, int> Bear::getPosition() const { 
    std::lock_guard<std::mutex> lock(mtx);
    return {x, y}; 
}

bool Bear::isAlive() const { 
    return alive.load(); 
}

void Bear::setAlive(bool is_alive) { 
    alive.store(is_alive); 
}

void Bear::moveRandomly(int map_width, int map_height) {
    if (!alive) return;
    
    std::lock_guard<std::mutex> lock(mtx);
    
    std::uniform_int_distribution<> move_dist(-getMoveDistance(), getMoveDistance());
    int dx = move_dist(gen);
    int dy = move_dist(gen);
    
    x = std::max(0, std::min(map_width - 1, x + dx));
    y = std::max(0, std::min(map_height - 1, y + dy));
}

bool Bear::canKill(const std::shared_ptr<NPC>& other) const {
    return other->getType() == "Werewolf";
}

int Bear::rollAttackDice() {
    return dice_dist(gen);
}

int Bear::rollDefenseDice() {
    return dice_dist(gen);
}

void Bear::addObserver(const std::shared_ptr<Observer>& observer) {
    observers.push_back(observer);
}

void Bear::notifyKill(const std::shared_ptr<NPC>& victim) {
    for (const auto& observer : observers) {
        observer->onKill(shared_from_this(), victim);
    }
}

void Bear::save(std::ostream& file) const {
    std::lock_guard<std::mutex> lock(mtx);
    bool is_alive = alive.load();
    file << "Bear " << name << " " << x << " " << y << " " << (is_alive ? 1 : 0) << "\n";
}