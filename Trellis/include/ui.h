#ifndef UI_H
#define UI_H
#include "imfilebrowser.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
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

    // Data
    std::string PageName = "";
    int         PageSize;

    int ActivePage     = 0;
    int PlayerPageView = 0;

    ~UI();
    UI();
    void Draw(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void DrawMenu(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void DrawPageSelect(Page::page_list_t &pages, Page::page_list_it_t &active_page);
    void DrawPageSettings(Page::page_list_it_t &active_page);

    void DrawClientList();

    Page::page_list_it_t GetActivePage(Page::page_list_t &pages);
    void                 ClearFlags();

    private:
    char PageNameBuf[128] = "";

    bool main_menu_open = true;
};
#endif