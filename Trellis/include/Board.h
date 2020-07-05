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

    Board(std::string name, uint64_t uid = 0);
    Board(const SQLite::Database &db, uint64_t uid, const std::string &name);
    ~Board() override;

    uint64_t Uid;

    void ClearPages();

    // GameState overrides
    void Update(float dt) override;
    void Draw() override;
    void RegisterKeyCallbacks() override;
    void UnregisterKeyCallbacks() override;
    void WriteToDB(const SQLite::Database &db) const final;

private:
    enum ClickType { NONE, PRESS, HOLD, RELEASE };

    UI                                                         UserInterface;
    std::string                                                Name;
    Page::page_list_t                                          Pages;
    Page::page_list_it_t                                       ActivePage = Pages.end();
    std::unordered_map<uint64_t, std::reference_wrapper<Page>> PagesMap;
    glm::ivec2                                                 MousePos;
    int                                                        ScrollDirection;
    ClickType                                                  LeftClick, RightClick, MiddleClick;
    Page::MouseHoverType                                       CurrentHoverType;

    void init_shaders();
    void init_objects();
    void set_projection();

    void ProcessUIEvents();
    void UpdateMouse();

    // Callbacks for networking
    void register_network_callbacks();
    void handle_page_add_piece(Data::NetworkData &&q);
    void handle_page_delete_piece(Data::NetworkData &&q);
    void handle_page_move_piece(Data::NetworkData &&q);
    void handle_page_resize_piece(Data::NetworkData &&q);
    void handle_new_image(Data::NetworkData &&q);
    void handle_client_join(Data::NetworkData &&q);
    void handle_add_page(Data::NetworkData &&q);

    // Host only command
    void handle_change_player_view(Data::NetworkData &&q);

    void AddPage(std::unique_ptr<Page> &&pg);
    void SendNewPage(const std::string &name);
    void SendUpdatedPage() const;
    void SendAllPages(uint64_t client_uid) const;

    void window_size_callback(int width, int height);
    void mouse_pos_callback(double x, double y);
    void scroll_callback(double yoffset);
    void snap_callback(int action);
    void esc_callback();

    void arrow_press(int key);
    void left_click_press();
    void left_click_release();
    void right_click_press();
    void right_click_release();
    void middle_click_press();
    void middle_click_release();
    void delete_press();

    void SetScreenDims(int width, int height);
};

#endif