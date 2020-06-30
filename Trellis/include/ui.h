#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imfilebrowser.h"
#include "imgui_stdlib.h"
#include "page.h"

#include <string>
#include <vector>

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
    void DrawMenu(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void DrawPageSelect(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void DrawPageSettings(Page::page_list_it_t &active_page);

    void DrawClientList();

    void ClearFlags();

private:
    std::string page_name_buf;

    bool main_menu_open = true;
};
#endif