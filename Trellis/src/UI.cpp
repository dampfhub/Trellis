#include "ui.h"

#include "client_server.h"
#include "data.h"
#include "glfw_handler.h"

#include <iostream>
#include <iterator>

using Data::ClientInfo;

UI::~UI() {
    delete FileDialog;
}

UI::UI() {
    FileDialog = new ImGui::FileBrowser(ImGuiFileBrowserFlags_CloseOnEsc);
}

void
UI::Draw(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    ImGui::ShowDemoWindow();
    // Display file dialog if it's open
    FileDialog->Display();
    DrawMenu(pages, active_page);
    DrawPageSelect(pages, active_page);
    DrawPageSettings(active_page);
    DrawClientList();
}

void
UI::DrawMenu(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    (void)pages;

    GLFW &glfw = GLFW::GetInstance();
    main_menu_open =
      ImGui::Begin("Trellis", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2(150.0f, (float)glfw.GetScreenHeight()));
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::Button("Add")) { ImGui::OpenPopup("add_menu"); }
    if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Add something to the page");
        ImGui::EndTooltip();
    }
    // Things that can be added
    if (ImGui::BeginPopup("add_menu")) {
        ImGui::Text("Add Options");
        ImGui::Separator();
        if (ImGui::Selectable("Add from File")) { FileDialog->Open(); }
        AddFromPreset = ImGui::Selectable("Add from Preset");
        if (ImGui::Selectable("Add Page##0")) { PageAddOpen = true; }
        ImGui::EndPopup();
    }

    // Page name popup
    if (PageAddOpen) { ImGui::OpenPopup("Add Page##1"); }
    if (ImGui::BeginPopupModal("Add Page##1", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Page Name: ");
        ImGui::SameLine();
        if (ImGui::InputText(
              "##edit",
              PageNameBuf,
              IM_ARRAYSIZE(PageNameBuf),
              ImGuiInputTextFlags_EnterReturnsTrue)) {
            ImGui::CloseCurrentPopup();
            PageName = PageNameBuf;
            strcpy(PageNameBuf, "");
            AddPage     = true;
            PageAddOpen = false;
        }
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            PageName = PageNameBuf;
            strcpy(PageNameBuf, "");
            AddPage     = true;
            PageAddOpen = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            PageAddOpen = false;
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Page Select")) { PageSelectOpen = !PageSelectOpen; }
    if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Select a page to view");
        ImGui::EndTooltip();
    }
    if (ImGui::Button("Page Settings")) {
        PageSettingsOpen = !PageSettingsOpen;
        strcpy(PageNameBuf, (**active_page).Name.c_str());
        PageSize = (int)(**active_page).board_transform.scale.x;
    }
    if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Change page settings");
        ImGui::EndTooltip();
    }
    ImGui::End();
}

void
UI::DrawPageSelect(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    if (!PageSelectOpen) { return; }
    ImGui::Begin(
      "Page Select",
      &PageSelectOpen,
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    int i = 0;
    ImGui::Text("Switch Page");
    ImGui::SameLine();
    ImGui::Text("Player View");
    for (auto &page : pages) {
        // Color the active page differently
        if (page == *active_page) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 1));
        }
        if (ImGui::Button(page->Name.c_str(), ImVec2(ImGui::GetWindowSize().x * 0.5f, 0.0f))) {
            ActivePage = i;
        }
        if (page == *active_page) { ImGui::PopStyleColor(1); }
        ImGui::SameLine();
        ImGui::RadioButton(("##" + std::to_string(i)).c_str(), &PlayerPageView, i);
        i++;
    }
    ImGui::End();
}

void
UI::DrawPageSettings(Page::page_list_it_t &active_page) {
    if (!PageSettingsOpen) { return; }
    Page &pg = **active_page;
    ImGui::Begin(
      "Page Settings",
      &PageSettingsOpen,
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Page Name: ");
    ImGui::InputText("##name", PageNameBuf, IM_ARRAYSIZE(PageNameBuf));

    ImGui::Text("Board Dimensions: ");
    ImGui::InputInt("X", &PageSize);
    if (ImGui::Button("Done", ImVec2(120, 0.0f))) {
        PageSettingsOpen           = false;
        pg.board_transform.scale.x = (float)PageSize;
        pg.board_transform.scale.y = (float)PageSize;
        pg.Name                    = PageNameBuf;
    }
    ImGui::End();
}

Page::page_list_it_t
UI::GetActivePage(Page::page_list_t &pages) {
    auto it = pages.begin();
    std::advance(it, ActivePage);
    return it;
}

void
UI::ClearFlags() {
    AddFromPreset = false;
    AddPage       = false;
    PageName      = "";
}

void
UI::DrawClientList() {
    static GLFW &glfw = GLFW::GetInstance();
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        if (main_menu_open) {
            if (cs.connected_clients.size() > 0) {
                ImGui::SetNextWindowSizeConstraints(
                  ImVec2(0, 50),
                  ImVec2(FLT_MAX, 50)); // Horizontal only
                ImGui::Begin(
                  "Clients",
                  nullptr,
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoMove);
                ImGui::SetWindowPos(ImVec2(150.0f, (float)glfw.GetScreenHeight() - 50));
                ImGui::SetWindowSize(ImVec2(100 * cs.connected_clients.size(), 50.0f));
                ImGui::BeginColumns(
                  "Columns",
                  cs.connected_clients.size(),
                  ImGuiColumnsFlags_NoResize);
                for (ClientInfo &inf : cs.connected_clients) {
                    ImGui::Text("%s", inf.Name.c_str());
                    ImGui::NextColumn();
                }
                ImGui::EndColumns();
                ImGui::End();
            }
        }
    }
}
