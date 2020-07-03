#include "state_manager.h"
#include "main_menu.h"

#include <stdexcept>

using std::make_unique, std::make_pair, std::string, std::runtime_error;

StateManager &
StateManager::GetInstance() {
    static StateManager instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

StateManager::StateManager()
    : main_menu(make_unique<MainMenu>())
    , current_state(*main_menu)
    , database("database.db") {
    current_state.get().RegisterKeyCallbacks();
    string error;
    int    result = database.Exec(
        "CREATE TABLE IF NOT EXISTS Games("
        "    id    INTEGER  UNIQUE PRIMARY KEY,"
        "    name  STRING   NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS Pages("
        "    id       INTEGER  UNIQUE PRIMARY KEY,"
        "    name     STRING   NOT NULL,"
        "    game_id  INTEGER  NOT NULL,"
        "    FOREIGN KEY(game_id) REFERENCES Games(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS GameObjects("
        "    id            INTEGER  UNIQUE PRIMARY KEY,"
        "    clickable     INTEGER  NOT NULL,"
        "    sprite        INTEGER  NOT NULL,"
        "    page_id       INTEGER  NOT NULL,"
        "    t_pos_x       REAL     NOT NULL,"
        "    t_pos_y       REAL     NOT NULL,"
        "    t_scale_x     REAL     NOT NULL,"
        "    t_scale_y     REAL     NOT NULL,"
        "    t_rotation    REAL     NOT NULL,"
        "    color_x       REAL     NOT NULL,"
        "    color_y       REAL     NOT NULL,"
        "    color_z       REAL     NOT NULL,"
        "    FOREIGN KEY(page_id)      REFERENCES Pages(id)"
        ");",
        error);
    if (result) { throw runtime_error(error); }
}

void
StateManager::Update(float dt) {
    current_state.get().Update(dt);
    current_state.get().WriteToDB(database);
}

void
StateManager::Draw() {
    current_state.get().Draw();
}

void
StateManager::StartNewGame(bool is_client) {
    auto b = make_unique<Board>();
    if (is_client) {
        b->Pages.clear();
        b->PagesMap.clear();
    }
    current_state.get().UnregisterKeyCallbacks();
    current_state = *b;
    current_state.get().RegisterKeyCallbacks();
    boards_map.insert(make_pair(b->Uid, std::move(b)));
}
