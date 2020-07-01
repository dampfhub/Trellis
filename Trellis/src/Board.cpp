#include "Board.h"

#include "GUI.h"
#include "client_server.h"
#include "data.h"
#include "game_object.h"
#include "glfw_handler.h"
#include "page.h"
#include "resource_manager.h"
#include "util.h"

#include <functional>
#include <iostream>

using std::unique_ptr, std::make_unique, std::move;

using Data::ImageData, Data::NetworkData;

void
Board::esc_handler() {
    if (ActivePage != Pages.end() && (*ActivePage)->Deselect()) { return; }
    static GLFW &glfw = GLFW::GetInstance();
    glfw.SetWindowShouldClose(1);
}

void
Board::window_size_callback(int width, int height) {
    SetScreenDims(width, height);
}

void
Board::mouse_pos_callback(double x, double y) {
    MousePos.x = x;
    MousePos.y = y;
}

void
Board::left_click_press() {
    LeftClick = Board::PRESS;
}

void
Board::left_click_release() {
    LeftClick = Board::RELEASE;
}

void
Board::right_click_press() {
    RightClick = Board::PRESS;
}

void
Board::right_click_release() {
    RightClick = Board::RELEASE;
}

void
Board::middle_click_press() {
    MiddleClick = Board::PRESS;
}

void
Board::middle_click_release() {
    MiddleClick = Board::RELEASE;
}

void
Board::scroll_callback(double yoffset) {
    ScrollDirection = (int)yoffset;
}

void
Board::arrow_press(int key) {
    if (ActivePage != Pages.end()) {
        switch (key) {
            case GLFW_KEY_RIGHT: (*ActivePage)->HandleArrows(Page::ArrowkeyType::RIGHT); break;
            case GLFW_KEY_LEFT: (*ActivePage)->HandleArrows(Page::ArrowkeyType::LEFT); break;
            case GLFW_KEY_DOWN: (*ActivePage)->HandleArrows(Page::ArrowkeyType::DOWN); break;
            case GLFW_KEY_UP: (*ActivePage)->HandleArrows(Page::ArrowkeyType::UP); break;
        }
    }
}

void
Board::delete_press() {
    if (ActivePage != Pages.end()) { (*ActivePage)->DeleteCurrentSelection(); }
}

void
Board::snap_callback(int action) {
    if (ActivePage != Pages.end()) {
        if (action == GLFW_PRESS) {
            (*ActivePage)->Snapping = false;
        } else if (action == GLFW_RELEASE) {
            (*ActivePage)->Snapping = true;
        }
    }
}

Board::Board()
    : Uid(Util::generate_uid()) {
    init_shaders();
    init_objects();
    // Set projection matrix
    set_projection();
    glm::mat4 view = glm::mat4(1.0f);
    ResourceManager::SetGlobalMatrix4("view", view);

    register_network_callbacks();
}

Board::~Board() {}

void
Board::RegisterKeyCallbacks() {
    static GLFW &glfw = GLFW::GetInstance();
    glfw.RegisterWindowSizeCallback(
        [this](int width, int height) { this->window_size_callback(width, height); });
    glfw.RegisterKeyPress(GLFW_KEY_ESCAPE, [this](int, int, int, int) { this->esc_handler(); });
    glfw.RegisterKeyPress(GLFW_KEY_RIGHT, [this](int key, int, int, int) {
        this->arrow_press(key);
    });
    glfw.RegisterKeyPress(GLFW_KEY_LEFT, [this](int key, int, int, int) {
        this->arrow_press(key);
    });
    glfw.RegisterKeyPress(GLFW_KEY_DOWN, [this](int key, int, int, int) {
        this->arrow_press(key);
    });
    glfw.RegisterKeyPress(GLFW_KEY_UP, [this](int key, int, int, int) { this->arrow_press(key); });
    glfw.RegisterMousePosCallback([this](double x, double y) { this->mouse_pos_callback(x, y); });
    glfw.RegisterScroll([this](double, double yoffset) { this->scroll_callback(yoffset); });
    glfw.RegisterMousePress(GLFW_MOUSE_BUTTON_LEFT, [this](int, int, int) {
        this->left_click_press();
    });
    glfw.RegisterMouseRelease(GLFW_MOUSE_BUTTON_LEFT, [this](int, int, int) {
        this->left_click_release();
    });
    glfw.RegisterMousePress(GLFW_MOUSE_BUTTON_RIGHT, [this](int, int, int) {
        this->right_click_press();
    });
    glfw.RegisterMouseRelease(GLFW_MOUSE_BUTTON_RIGHT, [this](int, int, int) {
        this->right_click_release();
    });
    glfw.RegisterMousePress(GLFW_MOUSE_BUTTON_MIDDLE, [this](int, int, int) {
        this->middle_click_press();
    });
    glfw.RegisterMouseRelease(GLFW_MOUSE_BUTTON_MIDDLE, [this](int, int, int) {
        this->middle_click_release();
    });
    glfw.RegisterKey(GLFW_KEY_LEFT_ALT, [this](int, int, int action, int) {
        this->snap_callback(action);
    });
    glfw.RegisterKeyPress(GLFW_KEY_DELETE, [this](int, int, int, int) { this->delete_press(); });
}

void
Board::UnregisterKeyCallbacks() {
    static GLFW &glfw = GLFW::GetInstance();
    glfw.UnregisterWindowSizeCallback();
    glfw.UnregisterKeyPress(GLFW_KEY_ESCAPE);
    glfw.UnregisterKeyPress(GLFW_KEY_RIGHT);
    glfw.UnregisterKeyPress(GLFW_KEY_LEFT);
    glfw.UnregisterKeyPress(GLFW_KEY_DOWN);
    glfw.UnregisterKeyPress(GLFW_KEY_UP);
    glfw.UnregisterMousePosCallback();
    glfw.UnregisterScroll();
    glfw.UnregisterMouse(GLFW_MOUSE_BUTTON_LEFT);
    glfw.UnregisterMouse(GLFW_MOUSE_BUTTON_RIGHT);
    glfw.UnregisterMouse(GLFW_MOUSE_BUTTON_MIDDLE);
    glfw.UnregisterKey(GLFW_KEY_LEFT_ALT);
    glfw.UnregisterKeyPress(GLFW_KEY_DELETE);
}

void
Board::SetScreenDims(int width, int height) {
    (void)width;
    (void)height;

    set_projection();
}

void
Board::init_shaders() {
    ResourceManager::LoadShader("shaders/sprite.vert", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/board.vert", "shaders/board.frag", nullptr, "board");
    ResourceManager::SetGlobalInteger("image", 0);
}

void
Board::init_objects() {
    SendNewPage("Default");
    ActivePage = Pages.begin();
}

void
Board::set_projection() {
    static GLFW &glfw       = GLFW::GetInstance();
    glm::mat4    projection = glm::ortho(
        0.0f,
        static_cast<float>(glfw.GetScreenWidth()),
        static_cast<float>(glfw.GetScreenHeight()),
        0.0f,
        -1.0f,
        1.0f);
    ResourceManager::SetGlobalMatrix4("projection", projection);
}

void
Board::UpdateMouse() {
    static GUI &gui = GUI::GetInstance();
    if (ActivePage == Pages.end()) { return; }
    Page &pg = **ActivePage;
    switch (LeftClick) {
        case PRESS:
            pg.HandleLeftClickPress(MousePos);
            current_hover_type = pg.CurrentHoverType(MousePos);
            LeftClick          = HOLD;
            break;
        case HOLD: pg.HandleLeftClickHold(MousePos); break;
        case RELEASE:
            pg.HandleLeftClickRelease(MousePos);
            current_hover_type = pg.CurrentHoverType(MousePos);
            break;
        default: break;
    }
    switch (RightClick) {
        case PRESS: pg.HandleRightClick(MousePos); break;
        default: break;
    }
    switch (MiddleClick) {
        case PRESS:
            pg.HandleMiddleClickPress(MousePos);
            MiddleClick = HOLD;
            break;
        case HOLD: pg.HandleMiddleClickHold(MousePos); break;
        case NONE:
        case RELEASE: break;
    }
    if (ScrollDirection != 0) {
        pg.HandleScrollWheel(MousePos, ScrollDirection);
        ScrollDirection = 0;
    }
    switch (current_hover_type) {
        case Page::MouseHoverType::CENTER: gui.SetCursor(ImGuiMouseCursor_Hand); break;
        case Page::MouseHoverType::E:
        case Page::MouseHoverType::W: gui.SetCursor(ImGuiMouseCursor_ResizeEW); break;
        case Page::MouseHoverType::N:
        case Page::MouseHoverType::S: gui.SetCursor(ImGuiMouseCursor_ResizeNS); break;
        case Page::MouseHoverType::NE:
        case Page::MouseHoverType::SW:
            // Put NESW cursor here if one exists, otherwise use hand cursor
        case Page::MouseHoverType::NW:
        case Page::MouseHoverType::SE:
            // Put NWSE cursor here if one exists, otherwise use hand cursor
            gui.SetCursor(ImGuiMouseCursor_Hand);
            break;
        case Page::MouseHoverType::NONE:
        default: gui.SetCursor(ImGuiMouseCursor_Arrow);
    }
}

void
Board::Update(float dt) {
    ProcessUIEvents();
    if (ActivePage != Pages.end()) {
        Page &pg = **ActivePage;
        pg.Update(MousePos);
    }
    UpdateMouse();
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.Update();
    }
}

void
Board::Draw() {
    if (ActivePage != Pages.end()) { (**ActivePage).Draw(); }
    UserInterface.Draw(Pages, ActivePage);
}

void
Board::ProcessUIEvents() {
    if (UserInterface.ActivePage == 0) {
        ActivePage = Pages.begin();
    } else {
        ActivePage = std::find_if(Pages.begin(), Pages.end(), [this](unique_ptr<Page> &pg) {
            return pg->Uid == UserInterface.ActivePage;
        });
    }
    if (UserInterface.FileDialog->HasSelected()) {
        std::string file_name =
            Util::PathBaseName(UserInterface.FileDialog->GetSelected().string());
        ResourceManager::LoadTexture(
            UserInterface.FileDialog->GetSelected().string().c_str(),
            Util::IsPng(file_name),
            file_name);
        (**ActivePage)
            .BeginPlacePiece(
                Transform(glm::vec2(0.0f, 0.0f), glm::vec2(98.0f, 98.0f), 0),
                ResourceManager::GetTexture(file_name));
        UserInterface.FileDialog->ClearSelected();
    }
    if (UserInterface.AddPage) {
        SendNewPage(UserInterface.PageName);
        // UserInterface.ActivePage = Pages.size() - 1;
    }
    if (UserInterface.SettingsPage) { SendUpdatedPage(); }
    UserInterface.ClearFlags();
}

void
Board::AddPage(std::unique_ptr<Page> &&pg) {
    PagesMap.insert(std::make_pair(pg->Uid, std::ref(*pg)));
    Pages.push_back(std::move(pg));
}

void
Board::SendNewPage(std::string name) {
    auto pg = std::make_unique<Page>(name);
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.RegisterPageChange("ADD_PAGE", pg->Uid, pg->Serialize());
    }
    AddPage(std::move(pg));
}

void
Board::SendUpdatedPage() {
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.RegisterPageChange("ADD_PAGE", (*ActivePage)->Uid, (*ActivePage)->Serialize());
    }
}

void
Board::handle_page_add_piece(NetworkData &&q) {
    static ClientServer &cs = ClientServer::GetInstance();
    auto                 g  = q.Parse<CoreGameObject>();
    auto obj = make_unique<GameObject>(g);
    if (ResourceManager::Images.find(obj->SpriteUid) == ResourceManager::Images.end()) {
        // Image isn't cached, need to request it
        cs.RegisterPageChange("IMAGE_REQUEST", cs.uid, obj->SpriteUid);
    } else {
        // Image is cached, just grab it from the resource manager
        obj->Sprite = ResourceManager::GetTexture(obj->SpriteUid);
    }
    // See if page exists and place piece new in it if it does
    auto it = PagesMap.find(q.Uid);
    if (it != PagesMap.end()) {
        Page &pg       = (*it).second;
        auto  piece_it = pg.PiecesMap.find(obj->Uid);
        // Only add piece if it doesn't already exist
        if (piece_it == pg.PiecesMap.end()) { pg.AddPiece(std::move(obj)); }
    }
}

void
Board::handle_page_move_piece(NetworkData &&q) {
    auto piece_data = q.Parse<NetworkData>();
    // Find the relevant page
    auto page_it = PagesMap.find(q.Uid);
    if (page_it != PagesMap.end()) {
        Page &pg = page_it->second;
        // If the page is found, find the relevant piece
        auto piece_it = pg.PiecesMap.find(piece_data.Uid);
        if (piece_it != pg.PiecesMap.end()) {
            GameObject &piece        = (*piece_it).second;
            piece.transform.position = piece_data.Parse<glm::vec2>();
        }
    }
}

void
Board::handle_page_resize_piece(NetworkData &&q) {
    auto piece_data = q.Parse<NetworkData>();
    // Find the relevant page
    auto page_it = PagesMap.find(q.Uid);
    if (page_it != PagesMap.end()) {
        Page &pg = page_it->second;
        // If the page is found, find the relevant piece
        auto piece_it = pg.PiecesMap.find(piece_data.Uid);
        if (piece_it != pg.PiecesMap.end()) {
            GameObject &piece     = (*piece_it).second;
            piece.transform.scale = piece_data.Parse<glm::vec2>();
        }
    }
}

void
Board::handle_new_image(NetworkData &&q) {
    ResourceManager::Images[q.Uid] = q.Parse<ImageData>();
    // Check which gameobjects need this texture and apply it.
    for (auto &pg : Pages) {
        for (auto &go : pg->Pieces) {
            if (go->SpriteUid == q.Uid) { go->Sprite = ResourceManager::GetTexture(q.Uid); }
        }
    }
}

void
Board::handle_client_join(NetworkData &&q) {
    static ClientServer &cs = ClientServer::GetInstance();
    SendAllPages(q.Uid);
}

void
Board::handle_page_delete_piece(NetworkData &&q) {
    auto piece_data = q.Parse<NetworkData>();
    // Find the relevant page
    auto page_it = PagesMap.find(q.Uid);
    if (page_it != PagesMap.end()) {
        Page &pg = page_it->second;
        // If the page is found, find the relevant piece
        auto piece_it = pg.PiecesMap.find(piece_data.Uid);
        if (piece_it != pg.PiecesMap.end()) { pg.DeletePiece((*piece_it).first); }
    }
}

void
Board::handle_add_page(NetworkData &&q) {
    auto pg      = CorePage::Deserialize(q.Data);
    auto page_it = PagesMap.find(q.Uid);
    if (page_it == PagesMap.end()) {
        AddPage(std::make_unique<Page>(std::move(pg)));
    } else {
        page_it->second.get() = pg;
    }
}

void
Board::handle_change_player_view(NetworkData &&q) {
    UserInterface.ActivePage = q.Uid;
}

void
Board::register_network_callbacks() {
    ClientServer &cs = ClientServer::GetInstance();
    cs.RegisterCallback("MOVE_PIECE", [this](NetworkData &&d) {
        handle_page_move_piece(std::move(d));
    });
    cs.RegisterCallback("ADD_PIECE", [this](NetworkData &&d) {
        handle_page_add_piece(std::move(d));
    });
    cs.RegisterCallback("DELETE_PIECE", [this](NetworkData &&d) {
        handle_page_delete_piece(std::move(d));
    });
    cs.RegisterCallback("RESIZE_PIECE", [this](NetworkData &&d) {
        handle_page_resize_piece(std::move(d));
    });
    cs.RegisterCallback("NEW_IMAGE", [this](NetworkData &&d) { handle_new_image(std::move(d)); });
    cs.RegisterCallback("JOIN", [this](NetworkData &&d) { handle_client_join(std::move(d)); });
    cs.RegisterCallback("ADD_PAGE", [this](NetworkData &&d) { handle_add_page(std::move(d)); });
    cs.RegisterCallback("PLAYER_VIEW", [this](NetworkData &&d) {
        handle_change_player_view(std::move(d));
    });
}

void
Board::SendAllPages(uint64_t client_uid) {
    if (ClientServer::Started()) {
        for (auto &pg : Pages) {
            ClientServer &cs = ClientServer::GetInstance();
            CorePage &core = *pg;
            cs.RegisterPageChange("ADD_PAGE", core.Uid, Util::serialize_vec(core), client_uid);
            pg->SendAllPieces(client_uid);
        }
    }
}
