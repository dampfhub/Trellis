#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "board.h"
#include "game_state.h"
#include "sqlite_handler.h"

#include <functional>
#include <unordered_map>

class StateManager {
public:
    StateManager(StateManager const &) = delete; // Disallow copying
    void operator=(StateManager const &) = delete;

    static StateManager &GetInstance();

    void Update(float dt);
    void Draw();

    void StartNewGame(bool is_client = false);

private:
    std::unique_ptr<GameState>                           main_menu;
    std::reference_wrapper<GameState>                    current_state;
    std::unordered_map<uint64_t, std::unique_ptr<Board>> boards_map;
    SQLite::Database                                     database;

    StateManager();
};

#endif
