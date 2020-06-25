#include "ui.h"
#include "glfw_handler.h"

#include <iostream>

UI::~UI() {
    delete FileDialog;
}

UI::UI() {
    FileDialog = new ImGui::FileBrowser(ImGuiFileBrowserFlags_CloseOnEsc);
}

void UI::Draw(std::vector<Page *> pages, Page *active_page) {
    ImGui::ShowDemoWindow();

    // Display file dialog if it's open
    FileDialog->Display();
    DrawMenu(pages, active_page);
    DrawPageSelect(pages, active_page);
    DrawPageSettings(active_page);
}

void UI::DrawMenu(std::vector<Page *> pages, Page *active_page) {
    (void)pages;

    GLFW &glfw = GLFW::GetInstance();
    ImGui::Begin(
            "Trellis",
            nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2(150.0f, (float)glfw.GetScreenHeight()));
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::Button("Add")) {
        ImGui::OpenPopup("add_menu");
    }
    if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Add something to the page");
        ImGui::EndTooltip();
    }
    // Things that can be added
    if (ImGui::BeginPopup("add_menu")) {
        ImGui::Text("Add Options");
        ImGui::Separator();
        if (ImGui::Selectable("Add from File")) {
            FileDialog->Open();
        }
        AddFromPreset = ImGui::Selectable("Add from Preset");
        if (ImGui::Selectable("Add Page##0")) {
            PageAddOpen = true;
        }
        ImGui::EndPopup();
    }

    // Page name popup
    if (PageAddOpen) {
        ImGui::OpenPopup("Add Page##1");
    }
    if (ImGui::BeginPopupModal(
            "Add Page##1", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
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
            AddPage = true;
            PageAddOpen = false;
        }
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            PageName = PageNameBuf;
            strcpy(PageNameBuf, "");
            AddPage = true;
            PageAddOpen = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            PageAddOpen = false;
        }
        ImGui::EndPopup();
    }
    if (ImGui::Button("Page Select")) {
        PageSelectOpen = !PageSelectOpen;
    }
    if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Select a page to view");
        ImGui::EndTooltip();
    }
    if (ImGui::Button("Page Settings")) {
        PageSettingsOpen = !PageSettingsOpen;
        strcpy(PageNameBuf, active_page->Name.c_str());
        PageSize = (int)active_page->Size.x;
    }
    if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("Change page settings");
        ImGui::EndTooltip();
    }
    ImGui::End();
}

void UI::DrawPageSelect(std::vector<Page *> pages, Page *active_page) {
    if (!PageSelectOpen) {
        return;
    }
    ImGui::Begin(
            "Page Select",
            &PageSelectOpen,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    int i = 0;
    ImGui::Text("Switch Page");
    ImGui::SameLine();
    ImGui::Text("Player View");
    for (Page *page : pages) {
        // Color the active page differently
        if (page == active_page) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 1));
        }
        if (ImGui::Button(
                page->Name.c_str(),
                ImVec2(ImGui::GetWindowSize().x * 0.5f, 0.0f))) {
            ActivePage = i;
        }
        if (page == active_page) {
            ImGui::PopStyleColor(1);
        }
        ImGui::SameLine();
        ImGui::RadioButton(("##" + std::to_string(i)).c_str(),
                &PlayerPageView,
                i);
        i++;
    }
    ImGui::End();
}

void UI::DrawPageSettings(Page *active_page) {
    if (!PageSettingsOpen) {
        return;
    }

    ImGui::Begin(
            "Page Settings",
            &PageSettingsOpen,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Page Name: ");
    ImGui::InputText(
            "##name", PageNameBuf, IM_ARRAYSIZE(PageNameBuf));

    ImGui::Text("Board Dimensions: ");
    ImGui::InputInt("X", &PageSize);
    if (ImGui::Button("Done", ImVec2(120, 0.0f))) {
        PageSettingsOpen = false;
        active_page->Size.x = (float)PageSize;
        active_page->Size.y = (float)PageSize;
        active_page->Renderer->Resize(PageSize);
        active_page->Name = PageNameBuf;
    }
    ImGui::End();
}

Page *UI::GetActivePage(std::vector<Page *> pages) {
    return pages[ActivePage];
}

void UI::ClearFlags() {
    AddFromPreset = false;
    AddPage = false;
    PageName = "";
}