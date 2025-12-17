#include "werewolf.h"
#include <iostream>
#include <random>
#include <fstream>

Werewolf::Werewolf(const std::string& name, int x, int y) 
    : name(name), x(x), y(y), alive(true) {}

void Werewolf::print() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Werewolf " << name << " at (" << x << ", " << y << ")" 
              << (alive ? " [ALIVE]" : " [DEAD]") << std::endl;
}

std::string Werewolf::getName() const { 
    std::lock_guard<std::mutex> lock(mtx);
    return name; 
}

std::string Werewolf::getType() const { 
    return "Werewolf"; 
}

std::pair<int, int> Werewolf::getPosition() const { 
    std::lock_guard<std::mutex> lock(mtx);
    return {x, y}; 
}

bool Werewolf::isAlive() const { 
    return alive.load(); 
}

void Werewolf::setAlive(bool is_alive) { 
    alive.store(is_alive); 
}

void Werewolf::moveRandomly(int map_width, int map_height) {
    if (!alive) return;
    
    std::lock_guard<std::mutex> lock(mtx);
    
    std::uniform_int_distribution<> move_dist(-getMoveDistance(), getMoveDistance());
    int dx = move_dist(gen);
    int dy = move_dist(gen);
    
    x = std::max(0, std::min(map_width - 1, x + dx));
    y = std::max(0, std::min(map_height - 1, y + dy));
}

bool Werewolf::canKill(const std::shared_ptr<NPC>& other) const {
    return other->getType() == "Bandit";
}

int Werewolf::rollAttackDice() {
    return dice_dist(gen);
}

int Werewolf::rollDefenseDice() {
    return dice_dist(gen);
}

void Werewolf::addObserver(const std::shared_ptr<Observer>& observer) {
    observers.push_back(observer);
}

void Werewolf::notifyKill(const std::shared_ptr<NPC>& victim) {
    for (const auto& observer : observers) {
        observer->onKill(shared_from_this(), victim);
    }
}

void Werewolf::save(std::ostream& file) const {
    std::lock_guard<std::mutex> lock(mtx);
    bool is_alive = alive.load();
    file << "Werewolf " << name << " " << x << " " << y << " " << (is_alive ? 1 : 0) << "\n";
}