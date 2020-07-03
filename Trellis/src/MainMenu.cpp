#include "main_menu.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_helpers.h"
#include "glfw_handler.h"
#include "GUI.h"
#include "imconfig.h"
#include "state_manager.h"
#include "client_server.h"

using namespace ImGui;

MainMenu::MainMenu() {
    clear_flags();
}

void
MainMenu::Update(float dt) {}

void
MainMenu::Draw() {
    GLFW &glfw = GLFW::GetInstance();
    GUI & gui  = GUI::GetInstance();
    // ImGui::ShowDemoWindow();
    SetNextWindowPos(
        ImVec2((float)glfw.GetScreenWidth() / 2, (float)glfw.GetScreenHeight() / 2),
        0,
        ImVec2(0.5f, 0.5f));
    SetNextWindowSize(ImVec2(500.0f, 500.0f));
    Begin(
        "MainMenu",
        nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
    {
        auto f = ImFontResource(gui.BigFont);
        auto c = ImStyleResource(ImGuiCol_Text, IM_COL32(34, 139, 34, 255));
        SetCursorPosX((((glm::vec2)GetWindowSize() - (glm::vec2)CalcTextSize("Trellis")) * 0.5f).x);
        Text("Trellis");
    }
    {
        auto f = ImFontResource(gui.MediumFont);
        Separator();
        Dummy(ImVec2(0.0f, 10.0f));
        // Menu Buttons
        auto c1 = ImStyleResource(ImGuiCol_Button, IM_COL32(1, 50, 32, 150));
        auto c2 = ImStyleResource(ImGuiCol_ButtonHovered, IM_COL32(1, 50, 32, 220));
        auto c3 = ImStyleResource(ImGuiCol_ButtonActive, IM_COL32(1, 50, 32, 255));
        if (starting_new_game) {
            // Start game button was pressed
            {
                auto c = ImStyleResource(ImGuiCol_Text, IM_COL32(34, 139, 34, 255));
                SetCursorPosX(
                    (((glm::vec2)GetWindowSize() - (glm::vec2)CalcTextSize("Start New Game")) *
                     0.5f)
                        .x);
                Text("Start New Game");
            }
            Dummy(ImVec2(0.0f, 10.0f));
            Text("Name");
            SameLine();
            InputText("##name", &client_name_buf);
            Dummy(ImVec2(0.0f, 10.0f));
            InputInt("Port", &port_buf);
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Create", glm::vec2(GetWindowSize().x, 0.0f))) { new_game(); }
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Back", glm::vec2(GetWindowSize().x, 0.0f))) { clear_flags(); }
        } else if (loading_game) {
            // Load game button was pressed
            // TODO when we have functionality to load things from disk
            {
                auto c = ImStyleResource(ImGuiCol_Text, IM_COL32(34, 139, 34, 255));
                SetCursorPosX(
                    (((glm::vec2)GetWindowSize() - (glm::vec2)CalcTextSize("Load Game")) * 0.5f).x);
                Text("Load Game");
            }
            if (Button("Back", glm::vec2(GetWindowSize().x, 0.0f))) { clear_flags(); }
        } else if (joining_game) {
            // Join game button was pressed
            {
                auto c = ImStyleResource(ImGuiCol_Text, IM_COL32(34, 139, 34, 255));
                SetCursorPosX(
                    (((glm::vec2)GetWindowSize() - (glm::vec2)CalcTextSize("Join Game")) * 0.5f).x);
                Text("Join Game");
            }
            Dummy(ImVec2(0.0f, 10.0f));
            Text("Client Name");
            SameLine();
            InputText("##name", &client_name_buf);
            Dummy(ImVec2(0.0f, 10.0f));
            Text("Hostname");
            SameLine();
            InputText("##host", &host_name_buf);
            Dummy(ImVec2(0.0f, 10.0f));
            InputInt("Port", &port_buf);
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Join", glm::vec2(GetWindowSize().x, 0.0f))) { join_game(); }
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Back", glm::vec2(GetWindowSize().x, 0.0f))) { clear_flags(); }
        } else {
            if (Button("New Game", glm::vec2(GetWindowSize().x, 0.0f))) {
                starting_new_game = true;
            }
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Load Game", glm::vec2(GetWindowSize().x, 0.0f))) { loading_game = true; }
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Join Game", glm::vec2(GetWindowSize().x, 0.0f))) { joining_game = true; }
            Dummy(ImVec2(0.0f, 10.0f));
            if (Button("Exit", glm::vec2(GetWindowSize().x, 0.0f))) { exit(); }
        }
    }
    End();
}

void
MainMenu::new_game() {
    StateManager &sm = StateManager::GetInstance();
    ClientServer &cs = ClientServer::GetInstance(ClientServer::SERVER);
    sm.StartNewGame();
    cs.Start(port_buf);
}

void
MainMenu::load_game() {}

void
MainMenu::join_game() {
    StateManager &sm = StateManager::GetInstance();
    ClientServer &cs = ClientServer::GetInstance(ClientServer::CLIENT);
    sm.StartNewGame(true);
    cs.Start(port_buf, client_name_buf, host_name_buf);
}

void
MainMenu::exit() {
    GLFW &glfw = GLFW::GetInstance();
    glfw.SetWindowShouldClose(1);
}

void
MainMenu::RegisterKeyCallbacks() {}

void
MainMenu::UnregisterKeyCallbacks() {}

void
MainMenu::clear_flags() {
    starting_new_game = false;
    loading_game      = false;
    joining_game      = false;
    client_name_buf   = "";
    host_name_buf     = "";
    // TODO only for testing purposes so don't have to retype
    host_name_buf = "localhost";
}
void
MainMenu::WriteToDB(const SQLite::Database &db) const {
    // No Main Menu state needs to be written to the database
    return;
}
