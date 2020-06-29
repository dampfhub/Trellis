#ifndef GAME_H
#define GAME_H

#include "client_server.h"
#include "data.h"
#include "page.h"
#include "ui.h"
#include "game_state.h"

#include <functional>
#include <glm/glm.hpp>
#include <list>
#include <unordered_map>
#include <vector>

class Board : public GameState {
public:
    Board(Board const &) = delete; // Disallow copying
    void operator=(Board const &) = delete;

    Board();
    ~Board();

    enum { NONE, PRESS, HOLD, RELEASE } LeftClick, RightClick, MiddleClick;
    glm::ivec2 MousePos;
    int        ScrollDirection;

    Page::page_list_t                                          Pages;
    Page::page_list_it_t                                       ActivePage = Pages.end();
    std::unordered_map<uint64_t, std::reference_wrapper<Page>> PagesMap;

    uint64_t Uid;

    // setters
    void SetScreenDims(int width, int height);

    // game loop
    void Update(float dt) override;

    void Draw() override;

    void RegisterKeyCallbacks() override;

    void UnregisterKeyCallbacks() override;

private:
    UI UserInterface;

    void init_shaders();

    void init_objects();

    void set_projection();

    void ProcessUIEvents();

    void UpdateMouse();

    void register_network_callbacks();

    // Callbacks for networking
    void handle_page_add_piece(Data::NetworkData &&q);

    void handle_page_delete_piece(Data::NetworkData &&q);

    void handle_page_move_piece(Data::NetworkData &&q);

    void handle_page_resize_piece(Data::NetworkData &&q);

    void handle_new_image(Data::NetworkData &&q);

    void handle_client_join(Data::NetworkData &&q);

    void handle_add_page(Data::NetworkData &&q);

    // Host only command
    void handle_change_player_view(Data::NetworkData &&q);

    Page::MouseHoverType current_hover_type = Page::MouseHoverType::NONE;

    void AddPage(std::unique_ptr<Page> &&pg);

    void SendNewPage(std::string name);

    void SendUpdatedPage();

    void SendAllPages(uint64_t client_uid);

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

    void delete_press();
};

#endif