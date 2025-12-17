#include "bandit.h"
#include <iostream>
#include <random>
#include <fstream>

Bandit::Bandit(const std::string& name, int x, int y) 
    : name(name), x(x), y(y), alive(true) {}

void Bandit::print() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Bandit " << name << " at (" << x << ", " << y << ")" 
              << (alive ? " [ALIVE]" : " [DEAD]") << std::endl;
}

std::string Bandit::getName() const { 
    std::lock_guard<std::mutex> lock(mtx);
    return name; 
}

std::string Bandit::getType() const { 
    return "Bandit"; 
}

std::pair<int, int> Bandit::getPosition() const { 
    std::lock_guard<std::mutex> lock(mtx);
    return {x, y}; 
}

bool Bandit::isAlive() const { 
    return alive.load(); 
}

void Bandit::setAlive(bool is_alive) { 
    alive.store(is_alive); 
}

void Bandit::moveRandomly(int map_width, int map_height) {
    if (!alive) return;
    
    std::lock_guard<std::mutex> lock(mtx);
    
    std::uniform_int_distribution<> move_dist(-getMoveDistance(), getMoveDistance());
    int dx = move_dist(gen);
    int dy = move_dist(gen);
    
    x = std::max(0, std::min(map_width - 1, x + dx));
    y = std::max(0, std::min(map_height - 1, y + dy));
}

bool Bandit::canKill(const std::shared_ptr<NPC>& other) const {
    return other->getType() == "Bear";
}

int Bandit::rollAttackDice() {
    return dice_dist(gen);
}

int Bandit::rollDefenseDice() {
    return dice_dist(gen);
}

void Bandit::addObserver(const std::shared_ptr<Observer>& observer) {
    observers.push_back(observer);
}

void Bandit::notifyKill(const std::shared_ptr<NPC>& victim) {
    for (const auto& observer : observers) {
        observer->onKill(shared_from_this(), victim);
    }
}

void Bandit::save(std::ostream& file) const {
    std::lock_guard<std::mutex> lock(mtx);
    bool is_alive = alive.load();
    file << "Bandit " << name << " " << x << " " << y << " " << (is_alive ? 1 : 0) << "\n";
}