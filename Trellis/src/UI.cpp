#include "ui.h"

#include "client_server.h"
#include "glfw_handler.h"
#include "imgui_helpers.h"

#include <iostream>
#include <iterator>
#include <ctime>

using Data::ClientInfo, Data::ChatMessage, Data::NetworkData;

using namespace ImGui;

UI::~UI() {
    delete FileDialog;
}

UI::UI() {
    FileDialog       = new FileBrowser(ImGuiFileBrowserFlags_CloseOnEsc);
    ClientServer &cs = ClientServer::GetInstance();
    cs.RegisterCallback("CHAT_MSG", [this](NetworkData &&d) { handle_chat_msg(std::move(d)); });
    cs.RegisterCallback("JOIN", [this](NetworkData &&d) { handle_client_join(std::move(d)); });
}

void
UI::Draw(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    // ShowDemoWindow();
    // Display file dialog if it's open
    FileDialog->Display();
    draw_menu(pages, active_page);
    draw_page_select(pages, active_page);
    draw_page_settings(active_page);
    draw_client_list();
    draw_chat();
}

void
UI::draw_menu(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    (void)pages;

    GLFW &glfw     = GLFW::GetInstance();
    main_menu_open = Begin("Trellis", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    SetWindowPos(ImVec2(0.0f, 0.0f));
    SetWindowSize(ImVec2(150.0f, (float)glfw.GetScreenHeight()));
    SetNextItemWidth(100.0f);
    if (Button("Add")) { OpenPopup("add_menu"); }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Add something to the page");
        EndTooltip();
    }
    // Things that can be added
    if (BeginPopup("add_menu")) {
        Text("Add Options");
        Separator();
        if (Selectable("Add from File")) { FileDialog->Open(); }
        AddFromPreset = Selectable("Add from Preset");
        if (Selectable("Add Page##0")) { PageAddOpen = true; }
        EndPopup();
    }

    // Page name popup
    if (PageAddOpen) { OpenPopup("Add Page##1"); }
    if (BeginPopupModal("Add Page##1", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        Text("Page Name: ");
        SameLine();
        if (InputText("##edit", &page_name_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
            CloseCurrentPopup();
            PageName      = page_name_buf;
            page_name_buf = "";
            AddPage       = true;
            PageAddOpen   = false;
        }
        Separator();
        if (Button("OK", ImVec2(120, 0))) {
            CloseCurrentPopup();
            PageName      = page_name_buf;
            page_name_buf = "";
            AddPage       = true;
            PageAddOpen   = false;
        }
        SameLine();
        if (Button("Cancel", ImVec2(120, 0))) {
            CloseCurrentPopup();
            PageAddOpen = false;
        }
        EndPopup();
    }
    if (Button("Page Select")) { PageSelectOpen = !PageSelectOpen; }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Select a page to view");
        EndTooltip();
    }
    if (Button("Page Settings")) {
        PageSettingsOpen = !PageSettingsOpen;
        page_name_buf    = (**active_page).Name;
        auto pg_dims     = (*active_page)->getCellDims();
        PageX            = pg_dims.x;
        PageY            = pg_dims.y;
    }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Change page settings");
        EndTooltip();
    }
    if (Button("Chat")) { chat_open = !chat_open; }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Chat with other players");
        EndTooltip();
    }
    End();
}

void
UI::draw_page_select(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    if (!PageSelectOpen) { return; }
    Begin(
        "Page Select",
        &PageSelectOpen,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    int i = 0;
    Text("Switch Page");
    SameLine();
    Text("Player View");
    for (auto &page : pages) {
        // Color the active page differently
        if (page == *active_page) { PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 1)); }
        if (Button(page->Name.c_str(), ImVec2(GetWindowSize().x * 0.5f, 0.0f))) {
            ActivePage = page->Uid;
        }
        if (page == *active_page) { PopStyleColor(1); }
        SameLine();
        if (RadioButton(("##" + std::to_string(i)).c_str(), &PlayerPageView, i)) {
            if (ClientServer::Started()) {
                static ClientServer &cs = ClientServer::GetInstance();
                cs.RegisterPageChange("PLAYER_VIEW", page->Uid, "");
            }
        }
        i++;
    }
    End();
}

void
UI::draw_page_settings(Page::page_list_it_t &active_page) {
    if (!PageSettingsOpen) { return; }
    Page &pg = **active_page;
    Begin(
        "Page Settings",
        &PageSettingsOpen,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    Text("Page Name: ");
    InputText("##name", &page_name_buf);

    Text("Board Dimensions: ");
    InputInt("X", &PageX);
    InputInt("Y", &PageY);
    if (Button("Done", ImVec2(120, 0.0f))) {
        PageSettingsOpen = false;
        pg.setCellDims(glm::ivec2(PageX, PageY));
        pg.Name      = page_name_buf;
        SettingsPage = true;
    }
    End();
}

void
UI::ClearFlags() {
    AddFromPreset = false;
    AddPage       = false;
    SettingsPage  = false;
    PageName      = "";
}

void
UI::draw_client_list() {
    static GLFW &glfw = GLFW::GetInstance();
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        if (main_menu_open) {
            if (cs.ConnectedClients.size() > 0) {
                SetNextWindowSizeConstraints(ImVec2(0, 50), ImVec2(FLT_MAX, 50)); // Horizontal only
                Begin(
                    "Clients",
                    nullptr,
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoMove);
                SetWindowPos(ImVec2(150.0f, (float)glfw.GetScreenHeight() - 50));
                SetWindowSize(ImVec2(100 * cs.ConnectedClients.size(), 50.0f));
                BeginColumns("Columns", cs.ConnectedClients.size(), ImGuiColumnsFlags_NoResize);
                for (ClientInfo &inf : cs.ConnectedClients) {
                    Text("%s", inf.Name.c_str());
                    NextColumn();
                }
                EndColumns();
                End();
            }
        }
    }
}

void
UI::draw_chat() {
    if (!chat_open) { return; }
    auto window_size = ImStyleResource(ImGuiStyleVar_WindowMinSize, ImVec2(300, 400));
    auto bg_col      = ImStyleResource(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 255));

    Begin("Chat", nullptr);
    SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once);
    {
        ImVec2 win_size = GetContentRegionAvail();
        win_size.y -= 20;
        BeginChild("##chat_text", ImVec2(win_size.x, win_size.y * 0.8));
        {
            std::string last_sender = "";
            for (auto &m : chat_messages) {
                if (m.SenderName != last_sender) {
                    {
                        auto c = ImStyleResource(ImGuiCol_Text, IM_COL32(34, 139, 34, 255));
                        Separator();
                        last_sender          = m.SenderName;
                        std::tm *t           = std::localtime(&m.TimeStamp);
                        auto     hr_standard = t->tm_hour % 12;
                        TextWrapped(
                            "%s - %d:%02d %s",
                            m.SenderName.c_str(),
                            hr_standard == 0 ? 12 : hr_standard,
                            t->tm_min,
                            t->tm_hour >= 12 ? "PM" : "AM");
                        Dummy(ImVec2(0.0f, 3.0f));
                    }
                    TextWrapped("%s", m.Msg.c_str());
                } else {
                    TextWrapped("%s\n", m.Msg.c_str());
                }
            }
            if (scroll_to_bottom) {
                SetScrollHere(1.0f);
                scroll_to_bottom = false;
            }
        }
        EndChild();
        if (input_needs_focus) {
            ImGui::SetKeyboardFocusHere(0);
            input_needs_focus = false;
        }
        if (InputTextMultiline(
                "##send_msg",
                &send_msg_buf,
                ImVec2(win_size.x, win_size.y * 0.15),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine)) {
            input_needs_focus = true;
            if (!send_msg_buf.empty()) {
                send_msg();
            }
        }
        if (Button("Send", ImVec2(win_size.x, 0)) && !send_msg_buf.empty()) { send_msg(); }
    }
    End();
}

void
UI::send_msg() {
    scroll_to_bottom        = true;
    static ClientServer &cs = ClientServer::GetInstance();
    ChatMessage          m(cs.Name, send_msg_buf);
    chat_messages.push_back(m);
    send_msg_buf = "";
    cs.RegisterPageChange("CHAT_MSG", m.Uid, m);
    // TODO Some sort of caching on the server side should probably happen here
}

void
UI::handle_chat_msg(Data::NetworkData &&q) {
    auto m = q.Parse<ChatMessage>();
    chat_messages.push_back(m);
    scroll_to_bottom = true;
}

void
UI::handle_client_join(NetworkData &&q) {
    static ClientServer &cs = ClientServer::GetInstance();
    for (auto &m : chat_messages) { cs.RegisterPageChange("CHAT_MSG", m.Uid, m, q.Uid); }
}
