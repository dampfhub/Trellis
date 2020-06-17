#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp>
#include <vector>

#include "page.h"

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE, GAME_MENU
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {
public:
    Game(Game const &) = delete; // Disallow copying
    void operator=(Game const &) = delete;

    static Game &GetInstance();

    // game state
    GameState State;
    enum {
        NONE, PRESS, HOLD, RELEASE
    } LeftClick, RightClick, MiddleClick;
    glm::ivec2 MousePos;
    int ScrollDirection;
    Page *ActivePage;
    std::vector<Page *> Pages;
    bool snapping = true;

    // setters
    void SetScreenDims(int width, int height);

    // game loop
    void Update(float dt);

    void Render();

private:
    Game();

    // destructor
    ~Game();

    void init_textures();

    void init_shaders();

    void init_objects();

    void set_projection();

    void ProcessUIEvents();

    void UpdateMouse();

    MouseHoverType current_hover_type = MouseHoverType::NONE;

    Page *MakePage(std::string name);
};

#endif