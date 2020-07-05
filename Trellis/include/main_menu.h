#ifndef MAIN_MENU_H
#define MAIN_MENU_H
#include "game_state.h"

#include <string>

class MainMenu : public GameState {
public:
    MainMenu();
    ~MainMenu() override = default;

    void Update() override;
    void Draw() override;
    void RegisterKeyCallbacks() override;
    void UnregisterKeyCallbacks() override;
    void WriteToDB(const SQLite::Database &db) const override;

private:
    bool starting_new_game = false;
    bool loading_game      = false;
    bool joining_game      = false;

    void new_game(const std::string &name) const;
    void load_game(uint64_t id, const std::string &name) const;
    void join_game() const;
    void exit() const;

    void clear_flags();

    std::string client_name_buf;
    std::string host_name_buf;
    int         port_buf = 5005;
};

#endif
