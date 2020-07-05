#include "Board.h"

#include "GUI.h"
#include "client_server.h"
#include "data.h"
#include "game_object.h"
#include "glfw_handler.h"
#include "page.h"
#include "resource_manager.h"
#include "util.h"
#include "state_manager.h"

#include <functional>
#include <iostream>
#include <utility>

using std::unique_ptr, std::make_unique, std::make_pair, std::ref, std::move, std::string,
    std::to_string, std::find_if;

using Data::ImageData, Data::NetworkData;

void
Board::esc_handler() {
    static StateManager &   sm   = StateManager::GetInstance();
    static GLFW &           glfw = GLFW::GetInstance();
    static ResourceManager &rm   = ResourceManager::GetInstance();

    if (ActivePage != Pages.end() && (*ActivePage)->Deselect()) { return; }
    // TODO: Check if this is a client or server, only save on server
    // TOOD: Also, move save initiation to a better location
    WriteToDB(sm.getDatabase());
    rm.WriteToDB(sm.getDatabase());

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

Board::Board(string name, uint64_t uid)
    : Name(std::move(name))
    , Uid(uid ? uid : Util::generate_uid()) {
    std::cout << "Starting \"" << Name << "\" with UID " << Uid << std::endl;
    init_shaders();
    init_objects();
    // Set projection matrix
    set_projection();
    glm::mat4        view = glm::mat4(1.0f);
    ResourceManager &rm   = ResourceManager::GetInstance();
    rm.SetGlobalMatrix4("view", view);

    register_network_callbacks();
}

Board::Board(const SQLite::Database &db, uint64_t uid, const std::string &name)
    : Board(name, uid) {
    using SQLite::from_uint64_t;
    using SQLite::to_uint64_t;

    auto page_callback = [](void *udp, int count, char **values, char **names) -> int {
        auto page_uids = static_cast<std::list<uint64_t> *>(udp);

        assert(!strcmp(names[0], "id"));
        page_uids->push_back(to_uint64_t(values[0]));
        return 0;
    };
    std::list<uint64_t> page_uids;
    std::string         err;
    int                 result = db.Exec(
        "SELECT id FROM Pages WHERE game_id = " + from_uint64_t(uid) + ";",
        err,
        +page_callback,
        &page_uids);
    if (result) { std::cerr << err << std::endl; }
    assert(!result);
    for (auto page_uid : page_uids) {
        auto page = make_unique<Page>(db, page_uid);
        AddPage(move(page));
    }
    auto stmt = db.Prepare("SELECT active_page FROM Games WHERE id = ?;");
    stmt.Bind(1, uid);
    stmt.Step();
    uint64_t active;
    stmt.Column(0, active);
    UserInterface.ActivePage = active;
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
    static ResourceManager &rm = ResourceManager::GetInstance();
    rm.LoadShader("shaders/sprite.vert", "shaders/sprite.frag", nullptr, "sprite");
    rm.LoadShader("shaders/board.vert", "shaders/board.frag", nullptr, "board");
    rm.SetGlobalInteger("image", 0);
}

void
Board::init_objects() {
    ActivePage = Pages.begin();
}

void
Board::set_projection() {
    static GLFW &           glfw       = GLFW::GetInstance();
    static ResourceManager &rm         = ResourceManager::GetInstance();
    glm::mat4               projection = glm::ortho(
        0.0f,
        static_cast<float>(glfw.GetScreenWidth()),
        static_cast<float>(glfw.GetScreenHeight()),
        0.0f,
        -1.0f,
        1.0f);
    rm.SetGlobalMatrix4("projection", projection);
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
}

void
Board::Draw() {
    if (ActivePage != Pages.end()) { (**ActivePage).Draw(); }
    UserInterface.Draw(Pages, ActivePage);
}

void
Board::ProcessUIEvents() {
    static ResourceManager &rm = ResourceManager::GetInstance();
    if (UserInterface.ActivePage == 0) {
        ActivePage = Pages.begin();
    } else {
        ActivePage = std::find_if(Pages.begin(), Pages.end(), [this](unique_ptr<Page> &pg) {
            return pg->Uid == UserInterface.ActivePage;
        });
    }
    if (UserInterface.FileDialog->HasSelected()) {
        string file_name = Util::PathBaseName(UserInterface.FileDialog->GetSelected().string());
        rm.LoadTexture(
            UserInterface.FileDialog->GetSelected().string().c_str(),
            Util::IsPng(file_name),
            file_name);
        (**ActivePage)
            .BeginPlacePiece(
                Transform(glm::vec2(0.0f, 0.0f), glm::vec2(98.0f, 98.0f), 0),
                rm.GetTexture(file_name));
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
Board::AddPage(unique_ptr<Page> &&pg) {
    PagesMap.insert(make_pair(pg->Uid, ref(*pg)));
    Pages.push_back(move(pg));
}

void
Board::AddPage(const CorePage &core_page) {
    auto page = make_unique<Page>(core_page);
    PagesMap.insert(make_pair(core_page.Uid, ref(*page)));
    Pages.push_back(move(page));
}

void
Board::SendNewPage(const string &name) {
    auto pg = make_unique<Page>(name);
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.RegisterPageChange("ADD_PAGE", pg->Uid, pg->Serialize());
    }
    AddPage(move(pg));
}

void
Board::SendUpdatedPage() const {
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.RegisterPageChange("ADD_PAGE", (*ActivePage)->Uid, (*ActivePage)->Serialize());
    }
}

void
Board::handle_page_add_piece(NetworkData &&q) {
    auto g = q.Parse<CoreGameObject>();
    // See if page exists and place piece new in it if it does
    auto it = PagesMap.find(q.Uid);
    if (it != PagesMap.end()) {
        Page &pg       = (*it).second;
        auto  piece_it = pg.PiecesMap.find(g.Uid);
        // Only add piece if it doesn't already exist
        if (piece_it == pg.PiecesMap.end()) { pg.AddPiece(g); }
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
    static ResourceManager &rm = ResourceManager::GetInstance();
    rm.Images[q.Uid]           = q.Parse<ImageData>();
    // Check which gameobjects need this texture and apply it.
    for (auto &pg : Pages) {
        for (auto &go : pg->Pieces) {
            if (go->SpriteUid == q.Uid) { go->Sprite = rm.GetTexture(q.Uid); }
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
    cs.RegisterCallback("JOIN", [this, &cs](NetworkData &&d) {
        cs.RegisterPageChange("JOIN_ACCEPT", this->Uid, this->Name, d.Uid);
        handle_client_join(std::move(d));
    });
    cs.RegisterCallback("ADD_PAGE", [this](NetworkData &&d) { handle_add_page(std::move(d)); });
    cs.RegisterCallback("PLAYER_VIEW", [this](NetworkData &&d) {
        handle_change_player_view(std::move(d));
    });
}

void
Board::SendAllPages(uint64_t client_uid) const {
    if (ClientServer::Started()) {
        for (auto &pg : Pages) {
            ClientServer &cs   = ClientServer::GetInstance();
            CorePage &    core = *pg;
            cs.RegisterPageChange("ADD_PAGE", core.Uid, Util::serialize_vec(core), client_uid);
            pg->SendAllPieces(client_uid);
        }
    }
}

void
Board::WriteToDB(const SQLite::Database &db) const {
    auto stmt = db.Prepare("INSERT OR REPLACE INTO Games VALUES(?,?,?);");
    stmt.Bind(1, Uid);
    stmt.Bind(2, Name);
    if (ActivePage == Pages.end()) {
        stmt.Bind(3);
    } else {
        stmt.Bind(3, ActivePage->get()->Uid);
    }
    stmt.Step();
    for (auto &page : Pages) { page->WriteToDB(db, Uid); }
}
