#include "game/game_manager.h"
#include <iostream>
#include <csignal>

//глобальный указатель на GameManager для обработки сигналов
GameManager* global_game_manager = nullptr;

//обработчик сигналов
void signalHandler(int signal) {
    if (global_game_manager) {
        std::cout << "\n\nReceived interrupt signal. Stopping game..." << std::endl;
        global_game_manager->stop();
    }
}

int main() {
    //устанавливаем обработчик сигналов
    std::signal(SIGINT, signalHandler);
    
    try {
        GameManager game;
        global_game_manager = &game;
        
        game.run();
        
        std::cout << "\nSimulation completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }
    
    global_game_manager = nullptr;
    return 0;
}