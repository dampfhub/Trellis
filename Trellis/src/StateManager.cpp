#include "state_manager.h"
#include "main_menu.h"

using std::make_unique, std::make_pair;

StateManager &
StateManager::GetInstance() {
    static StateManager instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

StateManager::StateManager()
    : main_menu(make_unique<MainMenu>())
    , current_state(*main_menu) {
    current_state.get().RegisterKeyCallbacks();
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
