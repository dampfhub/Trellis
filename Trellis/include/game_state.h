#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "sqlite_handler.h"

class GameState {
public:
    virtual ~GameState() {}
    virtual void Update(float dt)                            = 0;
    virtual void Draw()                                      = 0;
    virtual void RegisterKeyCallbacks()                      = 0;
    virtual void UnregisterKeyCallbacks()                    = 0;
    virtual void WriteToDB(const SQLite::Database &db) const = 0;
    // virtual void RegisterNetworkCallbacks() = 0;
};

#endif
