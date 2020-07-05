#include "state_manager.h"
#include "main_menu.h"

#include <stdexcept>

using std::make_unique, std::make_pair, std::string, std::runtime_error, std::unique_ptr;

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
        "    id           INTEGER  UNIQUE PRIMARY KEY,"
        "    name         STRING   NOT NULL,"
        "    active_page  INTEGER,"
        "    FOREIGN KEY(active_page) REFERENCES Pages(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS Pages("
        "    id          INTEGER  UNIQUE PRIMARY KEY,"
        "    name        STRING   NOT NULL,"
        "    game_id     INTEGER  NOT NULL,"
        "    t_pos_x     REAL     NOT NULL,"
        "    t_pos_y     REAL     NOT NULL,"
        "    t_scale_x   REAL     NOT NULL,"
        "    t_scale_y   REAL     NOT NULL,"
        "    t_rotation  REAL     NOT NULL,"
        "    cell_x      INTEGER  NOT NULL,"
        "    cell_y      INTEGER  NOT NULL,"
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
        "    FOREIGN KEY(page_id) REFERENCES Pages(id),"
        "    FOREIGN KEY(sprite)  REFERENCES Images(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS Images("
        "    id    INTEGER  UNIQUE PRIMARY KEY,"
        "    data  BLOB     NOT NULL"
        ");",
        error);
    if (result) { throw runtime_error(error); }
}

void
StateManager::WriteToDB(const SQLite::Database &db, [[maybe_unused]] const std::string &name)
    const {
    current_state.get().WriteToDB(db);
}

void
StateManager::Update() {
    current_state.get().Update();
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.Update();
    }
}

void
StateManager::Draw() {
    current_state.get().Draw();
}

void
StateManager::StartNewGame(const std::string &name, bool is_client, uint64_t uid, bool from_db) {
    unique_ptr<Board> b;
    if (from_db) {
        b = make_unique<Board>(database, uid, name);
    } else {
        b = make_unique<Board>(name, uid, is_client);
    }
    current_state.get().UnregisterKeyCallbacks();
    current_state = *b;
    current_state.get().RegisterKeyCallbacks();
    boards_map.insert(make_pair(b->Uid, std::move(b)));
}

const SQLite::Database &
StateManager::getDatabase() const {
    return database;
}
