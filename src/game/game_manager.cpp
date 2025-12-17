#include "game_manager.h"
#include "../factory/npc_factory.h"
#include "../observer/console_observer.h"
#include "../observer/file_observer.h"
#include "../utils/dice.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <sstream>

// инициализация статического члена
std::mutex GameManager::cout_mutex;

GameManager::GameManager() {
    // добавляем наблюдателей
    observers.push_back(std::make_shared<ConsoleObserver>());
    observers.push_back(std::make_shared<FileObserver>());
}

GameManager::~GameManager() {
    stop();
}

void GameManager::initialize() {
    safePrint("=== Balagur Fate 3 - Multi-threaded Simulation ===");
    safePrint("Variant 5: Bear, Werewolf, Bandit");
    safePrint("==================================================");
    safePrint("Initializing game...");
    safePrint("Map size: " + std::to_string(MAP_WIDTH) + "x" + std::to_string(MAP_HEIGHT));
    safePrint("Creating " + std::to_string(TOTAL_NPCS) + " NPCs...");
    
    addRandomNPCs(TOTAL_NPCS);
    
    // добавляем наблюдателей ко всем NPC
    for (auto& npc : npcs) {
        for (auto& observer : observers) {
            npc->addObserver(observer);
        }
    }
    
    safePrint("Game initialized successfully!");
    safePrint("Rules:");
    safePrint("  - Werewolf kills Bandit");
    safePrint("  - Bandit kills Bear");
    safePrint("  - Bear kills Werewolf");
    safePrint("  - Movement distances: Bear(5), Werewolf(40), Bandit(10)");
    safePrint("  - Kill distances: Bear(10), Werewolf(5), Bandit(10)");
    safePrint("==================================================");
}

void GameManager::start() {
    game_running = true;
    game_time = 0;
    total_battles = 0;
    total_kills = 0;
    
    // запускаем потоки с лямбда-функциями
    movement_thread = std::thread([this]() { movementWorker(); });
    battle_thread = std::thread([this]() { battleWorker(); });
    display_thread = std::thread([this]() { displayWorker(); });
    
    safePrint("Game started! Duration: " + std::to_string(GAME_DURATION) + " seconds");
}

void GameManager::stop() {
    game_running = false;
    battle_queue.stop();
    
    if (movement_thread.joinable()) movement_thread.join();
    if (battle_thread.joinable()) battle_thread.join();
    if (display_thread.joinable()) display_thread.join();
    
    battle_queue.clear();
}

void GameManager::run() {
    initialize();
    start();
    
    // ждём указанное время
    for (int i = 0; i < GAME_DURATION && game_running; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        game_time++;
    }
    
    stop();
    printFinalReport();
}

void GameManager::movementWorker() {
    safePrint("Movement thread started");
    
    while (game_running) {
        auto start_time = std::chrono::steady_clock::now();
        
        // блокировка для записи (уникальная)
        std::unique_lock<std::shared_mutex> lock(npcs_mutex);
        
        // двигаем всех живых NPC
        int moved_count = 0;
        for (auto& npc : npcs) {
            if (npc->isAlive()) {
                npc->moveRandomly(MAP_WIDTH, MAP_HEIGHT);
                moved_count++;
            }
        }
        
        // проверяем столкновения
        int collision_count = 0;
        std::set<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> checked_pairs;
        
        for (size_t i = 0; i < npcs.size(); ++i) {
            if (!npcs[i]->isAlive()) continue;
            
            for (size_t j = i + 1; j < npcs.size(); ++j) {
                if (!npcs[j]->isAlive()) continue;
                
                // проверяем, не проверяли ли мы уже эту пару
                auto pair = std::make_pair(npcs[i], npcs[j]);
                if (checked_pairs.count(pair)) continue;
                checked_pairs.insert(pair);
                
                double distance = npcs[i]->calculateDistance(npcs[j]);
                int kill_distance_i = npcs[i]->getKillDistance();
                int kill_distance_j = npcs[j]->getKillDistance();
                
                // NPC находятся в зоне убийства, если расстояние меньше любого из kill_distance
                if (distance <= kill_distance_i || distance <= kill_distance_j) {
                    // определяем, кто атакует (тот, кто может убить другого)
                    if (npcs[i]->canKill(npcs[j])) {
                        battle_queue.push({npcs[i], npcs[j], static_cast<int>(distance)});
                    } else if (npcs[j]->canKill(npcs[i])) {
                        battle_queue.push({npcs[j], npcs[i], static_cast<int>(distance)});
                    }
                    collision_count++;
                }
            }
        }
        
        lock.unlock();

        
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // регулируем скорость (примерно 10 обновлений в секунду)
        if (elapsed < std::chrono::milliseconds(100)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100) - elapsed);
        }
    }
    
    safePrint("Movement thread stopped");
}

void GameManager::battleWorker() {
    safePrint("Battle thread started");
    
    while (game_running) {
        BattleTask task;
        
        // получаем задачу из очереди с небольшим таймаутом
        if (battle_queue.pop(task, std::chrono::milliseconds(50))) {
            total_battles++;
            processBattle(task);
        }
        
        // небольшая пауза, чтобы не нагружать CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    safePrint("Battle thread stopped");
}

void GameManager::displayWorker() {
    safePrint("Display thread started");
    
    const int BAR_WIDTH = 40;
    
    while (game_running) {
        auto start_time = std::chrono::steady_clock::now();
        
        std::shared_lock<std::shared_mutex> lock(npcs_mutex);
        
        // вычисляем прогресс
        int progress = 0;
        if (GAME_DURATION > 0) {
            progress = (game_time * BAR_WIDTH) / GAME_DURATION;
        }
        if (progress > BAR_WIDTH) progress = BAR_WIDTH;
        
        // подсчет статистики
        int alive_count = countAliveNPCs();
        auto type_counts = countNPCsByType();
        
        // защита вывода
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        
        // компактный вывод с прогресс-баром
        std::cout << "\r["; // возврат каретки
        
        // прогресс-бар
        for (int i = 0; i < BAR_WIDTH; i++) {
            if (i < progress) std::cout << "=";
            else std::cout << ".";
        }
        
        // статистика
        std::cout << "] " 
          << std::setw(3) << game_time << "s/" 
          << std::setw(3) << GAME_DURATION << "s "
          << "Live:" << std::setw(3) << alive_count << "/" 
          << std::setw(3) << npcs.size()
          << " Br:" << std::setw(2) << type_counts["Bear"]
          << " Ww:" << std::setw(2) << type_counts["Werewolf"] 
          << " Bd:" << std::setw(2) << type_counts["Bandit"]
          << " Bat:" << std::setw(3) << total_battles.load()
          << " Kil:" << std::setw(2) << total_kills.load()
          << " Que:" << std::setw(3) << battle_queue.size()
          << std::flush;
        
        lock.unlock();
        
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // обновляем раз в секунду
        if (elapsed < std::chrono::seconds(1)) {
            std::this_thread::sleep_for(std::chrono::seconds(1) - elapsed);
        }
    }
    
    std::cout << std::endl; // перевод строки после завершения
    safePrint("Display thread stopped");
}

void GameManager::printMap() const {
    // упрощенная карта - можно убрать или оставить только для финального отчета
    const int DISPLAY_WIDTH = 40;
    const int DISPLAY_HEIGHT = 10;
    
    std::vector<std::vector<char>> map(DISPLAY_HEIGHT, 
                                       std::vector<char>(DISPLAY_WIDTH, '.'));
    
    // размещаем NPC на карте
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            auto [x, y] = npc->getPosition();
            int display_x = (x * DISPLAY_WIDTH) / MAP_WIDTH;
            int display_y = (y * DISPLAY_HEIGHT) / MAP_HEIGHT;
            
            if (display_x >= 0 && display_x < DISPLAY_WIDTH &&
                display_y >= 0 && display_y < DISPLAY_HEIGHT) {
                
                char symbol = '.';
                std::string type = npc->getType();
                
                if (type == "Bear") symbol = 'B';
                else if (type == "Werewolf") symbol = 'W';
                else if (type == "Bandit") symbol = 'R';
                
                map[display_y][display_x] = symbol;
            }
        }
    }
    
    std::cout << "\nSimplified Map (40x10):\n";
    std::cout << "  +";
    for (int j = 0; j < DISPLAY_WIDTH; ++j) std::cout << "-";
    std::cout << "+\n";
    
    for (int i = 0; i < DISPLAY_HEIGHT; ++i) {
        std::cout << "  |";
        for (int j = 0; j < DISPLAY_WIDTH; ++j) {
            std::cout << map[i][j];
        }
        std::cout << "|\n";
    }
    
    std::cout << "  +";
    for (int j = 0; j < DISPLAY_WIDTH; ++j) std::cout << "-";
    std::cout << "+\n";
}

void GameManager::printStatistics() const {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    
    std::cout << "\n=== CURRENT STATISTICS ===" << std::endl;
    std::cout << "Time: " << game_time.load() << "s" << std::endl;
    std::cout << "Battles: " << total_battles.load() 
              << "  Kills: " << total_kills.load() 
              << "  Queue: " << battle_queue.size() << std::endl;
    
    int alive_count = 0;
    std::map<std::string, int> type_counts;
    std::map<std::string, int> dead_counts;
    
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            alive_count++;
            type_counts[npc->getType()]++;
        } else {
            dead_counts[npc->getType()]++;
        }
    }
    
    std::cout << "\nAlive: " << alive_count << "/" << npcs.size() << std::endl;
    for (const auto& type : {"Bear", "Werewolf", "Bandit"}) {
        std::cout << type[0] << ":" << type_counts[type] 
                  << "/" << dead_counts[type] << "  ";
    }
    std::cout << std::endl;
}

void GameManager::printFinalReport() const {
    std::shared_lock<std::shared_mutex> lock(npcs_mutex);
    
    std::cout << "\n\n" << std::string(50, '=') << std::endl;
    std::cout << "           FINAL REPORT" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    std::cout << "Duration: " << game_time.load() << " seconds" << std::endl;
    std::cout << "Battles:  " << total_battles.load() << std::endl;
    std::cout << "Kills:    " << total_kills.load() << std::endl;
    
    int alive_count = 0;
    std::map<std::string, int> type_counts;
    std::map<std::string, int> dead_counts;
    
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            alive_count++;
            type_counts[npc->getType()]++;
        } else {
            dead_counts[npc->getType()]++;
        }
    }
    
    std::cout << "\nSurvivors: " << alive_count << "/" << npcs.size() 
              << " (" << std::fixed << std::setprecision(1) 
              << (alive_count * 100.0 / npcs.size()) << "%)" << std::endl;
    std::cout << std::string(30, '-') << std::endl;
    
    // компактная таблица результатов
    std::cout << "Type      Alive/Dead  Survival" << std::endl;
    
    for (const auto& type : {"Bear", "Werewolf", "Bandit"}) {
        int alive = type_counts[type];
        int dead = dead_counts[type];
        int total = alive + dead;
        double rate = total > 0 ? (alive * 100.0 / total) : 0.0;
        
        std::cout << std::left << std::setw(8) << type
                  << std::right << std::setw(4) << alive
                  << "/" << std::setw(4) << dead
                  << std::setw(10) << std::fixed << std::setprecision(1) << rate << "%"
                  << std::endl;
    }
    
    std::cout << std::string(50, '=') << std::endl;
    
    // список выживших (только первые 10 для компактности)
    std::cout << "\nTop 10 Survivors:" << std::endl;
    std::cout << std::string(30, '-') << std::endl;
    
    int survivor_num = 0;
    int printed = 0;
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            survivor_num++;
            if (printed < 10) {
                std::cout << survivor_num << ". " 
                          << npc->getType().substr(0, 1) 
                          << " " << npc->getName() 
                          << " at (" << npc->getPosition().first 
                          << "," << npc->getPosition().second << ")" 
                          << std::endl;
                printed++;
            }
        }
    }
    
    if (survivor_num > 10) {
        std::cout << "... and " << (survivor_num - 10) << " more" << std::endl;
    } else if (survivor_num == 0) {
        std::cout << "No survivors!" << std::endl;
    }
    
    std::cout << "\nDetailed log saved to 'log.txt'" << std::endl;
}

void GameManager::addRandomNPCs(int count) {
    npcs.clear();
    
    for (int i = 0; i < count; ++i) {
        std::string name = "NPC_" + std::to_string(i + 1);
        auto npc = NPCFactory::createRandomNPC(name, MAP_WIDTH, MAP_HEIGHT);
        npcs.push_back(npc);
    }
}

bool GameManager::checkCollision(const std::shared_ptr<NPC>& a, 
                                const std::shared_ptr<NPC>& b) const {
    if (!a->isAlive() || !b->isAlive()) return false;
    
    double distance = a->calculateDistance(b);
    return distance <= a->getKillDistance() || distance <= b->getKillDistance();
}

void GameManager::processBattle(const BattleTask& task) {
    if (!task.attacker->isAlive() || !task.defender->isAlive()) {
        return; // один из NPC уже мертв
    }
    
    resolveBattle(task.attacker, task.defender);
}

void GameManager::resolveBattle(std::shared_ptr<NPC> attacker, 
                               std::shared_ptr<NPC> defender) {
    // проверяем, может ли атакующий убить защитника
    if (!attacker->canKill(defender)) {
        // попробуем наоборот
        if (defender->canKill(attacker)) {
            std::swap(attacker, defender);
        } else {
            // никто никого не может убить - молча пропускаем
            return;
        }
    }
    
    // бросаем кубики
    int attack_roll = Dice::roll();
    int defense_roll = Dice::roll();
    
    if (attack_roll > defense_roll) {
        // убийство
        defender->setAlive(false);
        attacker->notifyKill(defender);
        total_kills++;
        
        // выводим только успешные убийства
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n[KILL] " << attacker->getType() << " " 
                  << attacker->getName() << " -> " 
                  << defender->getType() << " " << defender->getName()
                  << " (" << attack_roll << ">" << defense_roll << ")" 
                  << std::endl;
    }
    // неудачные атаки не выводим
}

void GameManager::safePrint(const std::string& message) const {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}

int GameManager::countAliveNPCs() const {
    int count = 0;
    for (const auto& npc : npcs) {
        if (npc->isAlive()) count++;
    }
    return count;
}

std::map<std::string, int> GameManager::countNPCsByType() const {
    std::map<std::string, int> counts;
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            counts[npc->getType()]++;
        }
    }
    return counts;
}