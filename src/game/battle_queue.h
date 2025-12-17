#ifndef BATTLE_QUEUE_H
#define BATTLE_QUEUE_H

#include "../npc/npc.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

struct BattleTask {
    std::shared_ptr<NPC> attacker;
    std::shared_ptr<NPC> defender;
    int distance;
    
    // конструктор по умолчанию
    BattleTask() : attacker(nullptr), defender(nullptr), distance(0) {}
    
    // конструктор с параметрами
    BattleTask(std::shared_ptr<NPC> a, std::shared_ptr<NPC> d, int dist = 0)
        : attacker(a), defender(d), distance(dist) {}
};

class BattleQueue {
private:
    std::queue<BattleTask> tasks;
    mutable std::mutex queue_mutex;
    std::condition_variable task_available;
    std::atomic<bool> running{true};
    
public:
    BattleQueue() = default;
    ~BattleQueue() { stop(); }
    
    // добавить задачу на бой
    void push(const BattleTask& task);
    void push(BattleTask&& task);
    
    // получить задачу (блокирующий вызов)
    bool pop(BattleTask& task);
    
    // получить задачу с таймаутом
    bool pop(BattleTask& task, std::chrono::milliseconds timeout);
    
    // проверить, пуста ли очередь
    bool empty() const;
    
    // получить размер очереди
    size_t size() const;
    
    // остановить очередь
    void stop();
    
    // очистить очередь
    void clear();
    
private:
    // удалить дублирующиеся задачи
    void removeDuplicates();
};

#endif