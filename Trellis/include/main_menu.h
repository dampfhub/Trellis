#ifndef MAIN_MENU_H
#define MAIN_MENU_H
#include "game_state.h"

class MainMenu : public GameState {
public:
    MainMenu();
    ~MainMenu() = default;

    void Update(float dt);
    void Draw();
    void RegisterKeyCallbacks();
    void UnregisterKeyCallbacks();

private:
    bool starting_new_game = false;
    bool loading_game      = false;
    bool joining_game      = false;

    void new_game();
    void load_game();
    void join_game();
    void exit();

    void clear_flags();

    char buf[256]  = {0};
    char hostname_buf[256] = {0};
    int  port_buf  = 5005;
};

#endif
