#ifndef GAME_H
#define GAME_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <list>
#include <functional>

#include "page.h"
#include "util.h"
#include "client_server.h"

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

    Page::page_list_t Pages;
    Page::page_list_it_t ActivePage = Pages.end();
    std::unordered_map<uint64_t, std::reference_wrapper<Page>> PagesMap;

    bool snapping = true;

    // setters
    void SetScreenDims(int width, int height);

    // game loop
    void Update(float dt);

    void Render();

private:
    Game();

    ~Game();

    void init_textures();

    void init_shaders();

    void init_objects();

    void set_projection();

    void ProcessUIEvents();

    void UpdateMouse();

    void start_server(int key, int scancode, int action, int mod);

    void start_client(int key, int scancode, int action, int mod);

    void register_network_callbacks();

    // Callbacks for networking
    void handle_page_add_piece(Util::NetworkData &&q);

    void handle_page_move_piece(Util::NetworkData &&q);

    void handle_page_resize_piece(Util::NetworkData &&q);

    void handle_new_image(Util::NetworkData &&q);

    void handle_image_request(Util::NetworkData &&q);

    Page::MouseHoverType current_hover_type = Page::MouseHoverType::NONE;

    void AddPage(std::unique_ptr<Page> &&pg);

    void MakePage(std::string name);

    void esc_handler();
    void window_size_callback(int width, int height);
    void mouse_pos_callback(double x, double y);
    void left_click_press();
    void left_click_release();
    void right_click_press();
    void right_click_release();
    void middle_click_press();
    void middle_click_release();
    void scroll_callback(double yoffset);
    void arrow_press(int key);
    void snap_callback(int action);
    void start_server_temp(int action);
    void start_client_temp(int action);
};

#endif