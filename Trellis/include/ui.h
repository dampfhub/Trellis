#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"
#include "imgui_stdlib.h"
#include "page.h"
#include "data.h"

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class UI {
public:
    ImGui::FileBrowser *FileDialog;

    // Flags
    bool PageSelectOpen   = false;
    bool PageSettingsOpen = false;
    bool PageAddOpen      = false;
    bool AddFromPreset    = false;
    bool AddPage          = false;
    bool SettingsPage     = false;

    // Data
    std::string PageName = "";
    int         PageX, PageY;

    uint64_t ActivePage     = 0;
    int      PlayerPageView = 0;

    ~UI();
    UI();
    void Draw(Page::page_list_t &pages, Page::page_list_it_t &active_page);

    void ClearFlags();

private:
    std::string                           page_name_buf;
    std::string                           send_msg_buf;
    std::string                           query_buf;
    nlohmann::json                        http_response;
    std::map<std::string, nlohmann::json> cached_results;
    std::vector<Data::ChatMessage>        chat_messages;

    bool main_menu_open   = true;
    bool scroll_to_bottom = false;
    bool chat_open        = true;
    bool http_window_open = true;

    void draw_main_node(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void draw_page_select(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void draw_page_settings(Page::page_list_it_t &active_page);
    void draw_chat();
    void draw_client_list();
    void draw_http_window();
    void draw_query_response(const std::string &query_type);

    void send_msg();

    void handle_chat_msg(Data::NetworkData &&q);

    void handle_client_join(Data::NetworkData &&q);
};
#endif