#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "game.h"

class StateManager {
public:
    StateManager(StateManager const &) = delete; // Disallow copying
    void operator=(StateManager const &) = delete;

    static StateManager &GetInstance();

private:
    StateManager() = default;
};

#endif
