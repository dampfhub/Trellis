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
        "    id  INTEGER  UNIQUE PRIMARY KEY"
        ");"
        "CREATE TABLE IF NOT EXISTS Pages("
        "    id       INTEGER  UNIQUE PRIMARY KEY,"
        "    name     STRING   NOT NULL,"
        "    game_id  INTEGER  NOT NULL,"
        "    FOREIGN KEY(game_id) REFERENCES Games(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS Vec2s("
        "    id  INTEGER  UNIQUE PRIMARY KEY,"
        "    x   REAL     NOT NULL,"
        "    y   REAL     NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS Vec3s("
        "    id  INTEGER  UNIQUE PRIMARY KEY,"
        "    x   REAL     NOT NULL,"
        "    y   REAL     NOT NULL,"
        "    z   REAL     NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS Transforms("
        "    id           INTEGER  UNIQUE PRIMARY KEY,"
        "    position_id  INTEGER  NOT NULL,"
        "    scale_id     INTEGER  NOT NULL,"
        "    FOREIGN KEY(position_id) REFERENCES Vec2s(id),"
        "    FOREIGN KEY(scale_id)    REFERENCES Vec2s(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS GameObjects("
        "    id            INTEGER  UNIQUE PRIMARY KEY,"
        "    clickable     INTEGER  NOT NULL,"
        "    sprite        INTEGER  NOT NULL,"
        "    page_id       INTEGER  NOT NULL,"
        "    transform_id  INTEGER  NOT NULL,"
        "    color_id      INTEGER  NOT NULL,"
        "    FOREIGN KEY(page_id)      REFERENCES Pages(id),"
        "    FOREIGN KEY(transform_id) REFERENCES Transforms(id),"
        "    FOREIGN KEY(color_id)     REFERENCES Vec3s(id)"
        ");",
        error);
    if (result) { throw runtime_error(error); }
}

void
StateManager::Update(float dt) {
    current_state.get().Update(dt);
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
