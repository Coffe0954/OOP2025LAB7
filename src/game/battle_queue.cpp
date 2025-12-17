#include "battle_queue.h"
#include <algorithm>
#include <chrono>

void BattleQueue::push(const BattleTask& task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(task);
    }
    task_available.notify_one();
}

void BattleQueue::push(BattleTask&& task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    task_available.notify_one();
}

bool BattleQueue::pop(BattleTask& task) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    
    // ждем, пока не появится задача или очередь не остановится
    task_available.wait(lock, [this]() {
        return !tasks.empty() || !running;
    });
    
    if (!running && tasks.empty()) {
        return false;
    }
    
    task = std::move(tasks.front());
    tasks.pop();
    
    return true;
}

bool BattleQueue::pop(BattleTask& task, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    
    // ждем с таймаутом
    if (!task_available.wait_for(lock, timeout, [this]() {
        return !tasks.empty() || !running;
    })) {
        return false; // таймаут
    }
    
    if (!running && tasks.empty()) {
        return false;
    }
    
    task = std::move(tasks.front());
    tasks.pop();
    
    return true;
}

bool BattleQueue::empty() const {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return tasks.empty();
}

size_t BattleQueue::size() const {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return tasks.size();
}

void BattleQueue::stop() {
    running = false;
    task_available.notify_all();
}

void BattleQueue::clear() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    while (!tasks.empty()) {
        tasks.pop();
    }
}

void BattleQueue::removeDuplicates() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    // временный вектор для хранения уникальных задач
    std::vector<BattleTask> unique_tasks;
    
    while (!tasks.empty()) {
        BattleTask task = std::move(tasks.front());
        tasks.pop();
        
        // проверяем, нет ли уже такой задачи
        auto it = std::find_if(unique_tasks.begin(), unique_tasks.end(),
            [&task](const BattleTask& t) {
                return (t.attacker == task.attacker && t.defender == task.defender) ||
                       (t.attacker == task.defender && t.defender == task.attacker);
            });
        
        if (it == unique_tasks.end()) {
            unique_tasks.push_back(std::move(task));
        }
    }
    
    // возвращаем уникальные задачи обратно в очередь
    for (auto& task : unique_tasks) {
        tasks.push(std::move(task));
    }
}