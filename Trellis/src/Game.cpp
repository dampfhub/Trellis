#include <iostream>

#include "game.h"
#include "glfw_handler.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "page.h"
#include "game_object.h"
#include "ui.h"
#include "util.h"
#include "GUI.h"
#include "client_server.h"

SpriteRenderer *ObjectRenderer;
UI *UserInterface;

Game &Game::GetInstance() {
    static Game instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

static void close_window(int key, int scancode, int action, int mods) {
    (void)key;
    (void)scancode;
    (void)action;
    (void)mods;

    static GLFW &glfw = GLFW::GetInstance();
    glfw.SetWindowShouldClose(1);
}

static void window_size_callback(int width, int height) {
    static Game &game = Game::GetInstance();
    game.SetScreenDims(width, height);
}

static void mouse_pos_callback(double x, double y) {
    static Game &game = Game::GetInstance();
    game.MousePos.x = x;
    game.MousePos.y = y;
}

static void left_click_press(int key, int action, int mod) {
    (void)key;
    (void)action;
    (void)mod;

    static Game &game = Game::GetInstance();
    game.LeftClick = Game::PRESS;
}

static void left_click_release(int key, int action, int mod) {
    (void)key;
    (void)action;
    (void)mod;

    static Game &game = Game::GetInstance();
    game.LeftClick = Game::RELEASE;
}

static void right_click_press(int key, int action, int mod) {
    (void)key;
    (void)action;
    (void)mod;

    static Game &game = Game::GetInstance();
    game.RightClick = Game::PRESS;
}

static void right_click_release(int key, int action, int mod) {
    (void)key;
    (void)action;
    (void)mod;

    static Game &game = Game::GetInstance();
    game.RightClick = Game::RELEASE;
}

static void middle_click_press(int key, int action, int mod) {
    (void)key;
    (void)action;
    (void)mod;

    static Game &game = Game::GetInstance();
    game.MiddleClick = Game::PRESS;
}

static void middle_click_release(int key, int action, int mod) {
    (void)key;
    (void)action;
    (void)mod;

    static Game &game = Game::GetInstance();
    game.MiddleClick = Game::RELEASE;
}

void scroll_callback(double xoffset, double yoffset) {
    (void)xoffset;

    static Game &game = Game::GetInstance();
    game.ScrollDirection = (int)yoffset;
}

void snap_callback(int key, int scancode, int action, int mod) {
    (void)key;
    (void)scancode;
    (void)mod;

    static Game &game = Game::GetInstance();
    if (action == GLFW_PRESS) {
        game.snapping = false;
    } else if (action == GLFW_RELEASE) {
        game.snapping = true;
    }
}

void start_server_temp(int key, int scancode, int action, int mod) {
    if (action == GLFW_PRESS) {
        ClientServer &cs = ClientServer::GetInstance(ClientServer::SERVER);
        Server *s = reinterpret_cast<Server *>(&cs);
        // TODO: Make this function virtual, always take 3 arguments, some defaulted
        s->Start(5005);
    }
}

void start_client_temp(int key, int scancode, int action, int mod) {
    if (action == GLFW_PRESS) {
        ClientServer &cs = ClientServer::GetInstance(ClientServer::CLIENT);
        Client *c = reinterpret_cast<Client *>(&cs);
        c->Start("Test Client", "localhost", 5005);
    }
}

Game::Game() {
    GLFW &glfw = GLFW::GetInstance();
    init_shaders();
    init_textures();
    init_objects();

    glfw.RegisterWindowSizeCallback(window_size_callback);
    glfw.RegisterKeyPress(GLFW_KEY_ESCAPE, close_window);
    glfw.RegisterMousePosCallback(mouse_pos_callback);
    glfw.RegisterScroll(scroll_callback);
    glfw.RegisterMousePress(GLFW_MOUSE_BUTTON_LEFT, left_click_press);
    glfw.RegisterMouseRelease(GLFW_MOUSE_BUTTON_LEFT, left_click_release);
    glfw.RegisterMousePress(GLFW_MOUSE_BUTTON_RIGHT, right_click_press);
    glfw.RegisterMouseRelease(GLFW_MOUSE_BUTTON_RIGHT, right_click_release);
    glfw.RegisterMousePress(GLFW_MOUSE_BUTTON_MIDDLE, middle_click_press);
    glfw.RegisterMouseRelease(GLFW_MOUSE_BUTTON_MIDDLE, middle_click_release);
    glfw.RegisterKey(GLFW_KEY_LEFT_ALT, snap_callback);
    glfw.RegisterKey(GLFW_KEY_A, start_server_temp);
    glfw.RegisterKey(GLFW_KEY_S, start_client_temp);

    // Set projection matrix
    set_projection();
    glm::mat4 view = glm::mat4(1.0f);
    ResourceManager::GetShader("sprite").SetMatrix4("view", view, true);
}

Game::~Game() {
    delete ObjectRenderer;
    delete UserInterface;
}

void Game::SetScreenDims(int width, int height) {
    (void)width;
    (void)height;

    set_projection();
}

void Game::init_shaders() {
    ResourceManager::LoadShader(
            "shaders/sprite.vert", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::GetShader("sprite").SetInteger("image", 0, true);
}

void Game::init_textures() {
    ResourceManager::LoadTexture("textures/grid.png", false, "grid");
    ResourceManager::LoadTexture("textures/token.jpg", false, "goblin");
    ResourceManager::LoadTexture("textures/orcling.png", true, "orcling");
}

void Game::init_objects() {
    ObjectRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    std::unique_ptr<SpriteRenderer>
            BoardRenderer = std::make_unique<SpriteRenderer>(
            ResourceManager::GetShader("sprite"), 20);
    UserInterface = new UI();
    MakePage("Default");
    ActivePage = Pages.begin();
}

void Game::set_projection() {
    static GLFW &glfw = GLFW::GetInstance();
    glm::mat4 projection = glm::ortho(
            0.0f,
            static_cast<float>(glfw.GetScreenWidth()),
            static_cast<float>(glfw.GetScreenHeight()),
            0.0f,
            -1.0f,
            1.0f);
    ResourceManager::GetShader("sprite").SetMatrix4(
            "projection", projection, true);
}

void Game::UpdateMouse() {
    static GUI &gui = GUI::GetInstance();
    Page &pg = **ActivePage;
    switch (LeftClick) {
        case PRESS:
            pg.HandleLeftClickPress(MousePos);
            current_hover_type = pg.CurrentHoverType(MousePos);
            LeftClick = HOLD;
            break;
        case HOLD:
            pg.HandleLeftClickHold(MousePos);
            break;
        case RELEASE:
            pg.HandleLeftClickRelease(MousePos);
            current_hover_type = pg.CurrentHoverType(MousePos);
            break;
        default:
            break;
    }
    switch (RightClick) {
        case PRESS:
            pg.HandleRightClick(MousePos);
            break;
        default:
            break;
    }
    switch (MiddleClick) {
        case PRESS:
            pg.HandleMiddleClickPress(MousePos);
            MiddleClick = HOLD;
            break;
        case HOLD:
            pg.HandleMiddleClickHold(MousePos);
            break;
        case NONE:
        case RELEASE:
            break;
    }
    if (ScrollDirection != 0) {
        pg.HandleScrollWheel(MousePos, ScrollDirection);
        ScrollDirection = 0;
    }
    switch (current_hover_type) {
        case CENTER:
            gui.SetCursor(ImGuiMouseCursor_Hand);
            break;
        case E:
        case W:
            gui.SetCursor(ImGuiMouseCursor_ResizeEW);
            break;
        case N:
        case S:
            gui.SetCursor(ImGuiMouseCursor_ResizeNS);
            break;
        case NE:
        case SW:
            //Put NESW cursor here if one exists, otherwise use hand cursor
        case NW:
        case SE:
            //Put NWSE cursor here if one exists, otherwise use hand cursor
            gui.SetCursor(ImGuiMouseCursor_Hand);
            break;
        case NONE:
        default:
            gui.SetCursor(ImGuiMouseCursor_Arrow);
    }
}

bool registered = false;

void Game::Update(float dt) {
    ProcessUIEvents();
    Page &pg = **ActivePage;
    pg.Update(dt);
    if (pg.Placing) {
        pg.UpdatePlacing(MousePos);
    }
    UpdateMouse();
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        cs.Update();
        if (!registered) {
            register_network_callbacks();
            registered = true;
        }
    }
}

void Game::Render() {
    (**ActivePage).Draw(ObjectRenderer, nullptr);
    UserInterface->Draw(Pages, ActivePage);
}

void Game::ProcessUIEvents() {
    ActivePage = UserInterface->GetActivePage(Pages);
    if (UserInterface->FileDialog->HasSelected()) {
        std::string file_name = Util::PathBaseName(
                UserInterface->FileDialog->GetSelected().string());
        ResourceManager::LoadTexture(
                UserInterface->FileDialog->GetSelected().string().c_str(),
                Util::IsPng(file_name),
                file_name);
        (**ActivePage).BeginPlacePiece(
                std::make_unique<GameObject>(
                        glm::vec2(0.0f, 0.0f),
                        glm::vec2(98.0f, 98.0f),
                        ResourceManager::GetTexture(file_name)));
        UserInterface->FileDialog->ClearSelected();
    }
    if (UserInterface->AddPage) {
        MakePage(UserInterface->PageName);
        UserInterface->ActivePage = Pages.size() - 1;
    }
    UserInterface->ClearFlags();
}

void Game::MakePage(std::string name) {
    auto BoardRenderer = std::make_unique<SpriteRenderer>(
            ResourceManager::GetShader("sprite"), 20);
    auto pg = std::make_unique<Page>(
            name,
            ResourceManager::GetTexture("grid"),
            std::move(BoardRenderer),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(20.0f, 20.0f));
    PagesMap.insert(std::make_pair(pg->Uid, std::ref(*pg)));
    Pages.push_back(std::move(pg));
}

void Game::start_server(int key, int scancode, int action, int mod) {
    ClientServer &cs = ClientServer::GetInstance(ClientServer::SERVER);
    register_network_callbacks();
}

void Game::start_client(int key, int scancode, int action, int mod) {
    ClientServer &cs = ClientServer::GetInstance(ClientServer::CLIENT);
    register_network_callbacks();
}

void Game::handle_page_add_piece(Util::NetworkData &&q) {
    static ClientServer &cs = ClientServer::GetInstance();
    auto g = std::make_unique<GameObject>(q.Parse<GameObject>());
    if (ResourceManager::Images.find(g->Sprite.ImageUID) ==
            ResourceManager::Images.end()) {
        // Image isn't cached, need to request it
        cs.RegisterPageChange("IMAGE_REQUEST", g->Sprite.ImageUID, "");
    }
    // See if page exists and place piece new in it if it does
    auto it = PagesMap.find(q.Uid);
    if (it != PagesMap.end()) {
        Page &pg = (*it).second;
        pg.PiecesMap.insert(std::make_pair(g->Uid, std::ref(*g)));
        pg.Pieces.push_front(std::move(g));
    }
}

void Game::handle_page_move_piece(Util::NetworkData &&q) {
    auto piece_data = q.Parse<Util::NetworkData>();
    // Find the relevant page
    auto page_it = PagesMap.find(q.Uid);
    if (page_it != PagesMap.end()) {
        Page &pg = page_it->second;
        // If the page is found, find the relevant piece
        auto piece_it = pg.PiecesMap.find(piece_data.Uid);
        if (piece_it != pg.PiecesMap.end()) {
            GameObject &piece = (*piece_it).second;
            piece.Position = piece_data.Parse<glm::vec2>();
        }
    }
}

void Game::handle_page_resize_piece(Util::NetworkData &&q) {
    auto piece_data = q.Parse<Util::NetworkData>();
    // Find the relevant page
    auto page_it = PagesMap.find(q.Uid);
    if (page_it != PagesMap.end()) {
        Page &pg = page_it->second;
        // If the page is found, find the relevant piece
        auto piece_it = pg.PiecesMap.find(piece_data.Uid);
        if (piece_it != pg.PiecesMap.end()) {
            GameObject &piece = (*piece_it).second;
            piece.Size = piece_data.Parse<glm::vec2>();
        }
    }
}

void Game::handle_new_image(Util::NetworkData &&q) {
    ResourceManager::Images[q.Uid] = q.Parse<ImageData>();
    // Check which gameobjects need this texture and apply it.
    for (auto &pg : Pages) {
        for (auto &go : pg->Pieces) {
            if (go->Sprite.ImageUID == q.Uid) {
                go->Sprite = ResourceManager::GetTexture(q.Uid);
            }
        }
    }
}

void Game::handle_image_request(Util::NetworkData &&q) {
    static ClientServer &cs = ClientServer::GetInstance();
    if (ResourceManager::Images.find(q.Uid) != ResourceManager::Images.end()) {
        cs.RegisterPageChange(
                "NEW_IMAGE", q.Uid, ResourceManager::Images[q.Uid]);
    }
}

void Game::register_network_callbacks() {
    ClientServer &cs = ClientServer::GetInstance();
    cs.RegisterCallback(
            "MOVE_PIECE", [this](Util::NetworkData &&d) {
                handle_page_move_piece(std::move(d));
            });
    cs.RegisterCallback(
            "ADD_PIECE", [this](Util::NetworkData &&d) {
                handle_page_add_piece(std::move(d));
            });
    cs.RegisterCallback(
            "RESIZE_PIECE", [this](Util::NetworkData &&d) {
                handle_page_resize_piece(std::move(d));
            });
    cs.RegisterCallback(
            "NEW_IMAGE", [this](Util::NetworkData &&d) {
                handle_new_image(std::move(d));
            });
    cs.RegisterCallback(
            "IMAGE_REQUEST", [this](Util::NetworkData &&d) {
                handle_image_request(std::move(d));
            });
}

