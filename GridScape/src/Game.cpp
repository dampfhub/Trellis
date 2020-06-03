#include <iostream>

#include "game.h"
#include "glfw_handler.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "page.h"
#include "game_object.h"
#include "ui.h"
#include "util.h"
#include "gui.h"

SpriteRenderer *ObjectRenderer;
UI *UserInterface;

Game &Game::GetInstance() {
    static Game instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

static void close_window(int key, int scancode, int action, int mods) {
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
    static Game &game = Game::GetInstance();
    game.LeftClick = Game::PRESS;
}

static void left_click_release(int key, int action, int mod) {
    static Game &game = Game::GetInstance();
    game.LeftClick = Game::RELEASE;
}

static void right_click_press(int key, int action, int mod) {
    static Game &game = Game::GetInstance();
    game.RightClick = Game::PRESS;
}

static void right_click_release(int key, int action, int mod) {
    static Game &game = Game::GetInstance();
    game.RightClick = Game::RELEASE;
}

static void middle_click_press(int key, int action, int mod) {
    static Game &game = Game::GetInstance();
    game.MiddleClick = Game::PRESS;
}

static void middle_click_release(int key, int action, int mod) {
    static Game &game = Game::GetInstance();
    game.MiddleClick = Game::RELEASE;
}

void scroll_callback(double xoffset, double yoffset) {
    static Game &game = Game::GetInstance();
    game.ScrollDirection = (int)yoffset;
}

Game::Game() {
    GLFW &glfw = GLFW::GetInstance();
    this->init_shaders();
    this->init_textures();
    this->init_objects();

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

    // Set projection matrix
    this->set_projection();
    glm::mat4 view = glm::mat4(1.0f);
    ResourceManager::GetShader("sprite").SetMatrix4("view", view, true);
}

Game::~Game() {
    for (Page *page : this->Pages) {
        delete page;
    }
    delete ObjectRenderer;
    delete UserInterface;
}

void Game::SetScreenDims(int width, int height) {
    this->set_projection();
}

void Game::init_shaders() {
    ResourceManager::LoadShader(
            "shaders/sprite.vert", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::GetShader("sprite").SetInteger("image", 0, true);
}

void Game::init_textures() {
    ResourceManager::LoadTexture("textures/grid.png", false, "grid");
    ResourceManager::LoadTexture("textures/token.png", false, "goblin");
    ResourceManager::LoadTexture("textures/orcling.png", true, "orcling");
}

void Game::init_objects() {
    ObjectRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    SpriteRenderer *BoardRenderer =
            new SpriteRenderer(ResourceManager::GetShader("sprite"), 20);
    UserInterface = new UI();
    this->ActivePage = new Page(
            "Default",
            ResourceManager::GetTexture("grid"),
            BoardRenderer,
            glm::vec2(0.0f, 0.0f),
            glm::vec2(20.0f, 20.0f));
    this->Pages.push_back(this->ActivePage);
    GameObject *test = new GameObject(
            glm::vec2(1.0f, 1.0f),
            glm::vec2(98.0f, 98.0f),
            ResourceManager::GetTexture("goblin"));
    GameObject *orc = new GameObject(
            glm::vec2(2.0f, 1.0f),
            glm::vec2(98.0f, 98.0f),
            ResourceManager::GetTexture("orcling"));
    this->ActivePage->PlacePiece(test);
    this->ActivePage->PlacePiece(orc);
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
    if (this->LeftClick != HOLD) {
        this->current_hover_type = this->ActivePage->MouseHoverSelection(this->MousePos);
    }
    switch (this->current_hover_type) {
        case CENTER:
            gui.SetCursor(ImGuiMouseCursor_Hand);
            break;
        case EW:
            gui.SetCursor(ImGuiMouseCursor_ResizeEW);
            break;
        case NS:
            gui.SetCursor(ImGuiMouseCursor_ResizeNS);
            break;
        case NESW:
            gui.SetCursor(ImGuiMouseCursor_Hand);
            break;
        case NWSE:
            gui.SetCursor(ImGuiMouseCursor_Hand);
            break;
        default:
            gui.SetCursor(ImGuiMouseCursor_Arrow);
    }
    switch (this->LeftClick) {
        case PRESS:
            this->ActivePage->HandleLeftClickPress(this->MousePos);
            this->LeftClick = HOLD;
            break;
        case HOLD:
            this->ActivePage->HandleLeftClickHold(this->MousePos);
            break;
        case RELEASE:
            this->ActivePage->HandleLeftClickRelease(this->MousePos);
            break;
        default:
            break;
    }
    switch (this->RightClick) {
        case PRESS:
            this->ActivePage->HandleRightClick(this->MousePos);
            break;
        default:
            break;
    }
    switch (this->MiddleClick) {
        case PRESS:
            this->ActivePage->HandleMiddleClickPress(this->MousePos);
            this->MiddleClick = HOLD;
            break;
        case HOLD:
            this->ActivePage->HandleMiddleClickHold(this->MousePos);
    }
    if (this->ScrollDirection != 0) {
        this->ActivePage
                ->HandleScrollWheel(this->MousePos, this->ScrollDirection);
        this->ScrollDirection = 0;
    }
}

void Game::Update(float dt) {
    this->ProcessUIEvents();
    this->ActivePage->Update(dt);
    if (this->ActivePage->Placing) {
        this->ActivePage->UpdatePlacing(this->MousePos);
    }
    this->UpdateMouse();
}

void Game::Render() {
    this->ActivePage->Draw(ObjectRenderer, nullptr);
    UserInterface->Draw(this->Pages, this->ActivePage);
}

void Game::ProcessUIEvents() {
    this->ActivePage = UserInterface->GetActivePage(this->Pages);
    if (UserInterface->FileDialog->HasSelected()) {
        std::string file_name = Util::PathBaseName(
                UserInterface->FileDialog->GetSelected().string());
        ResourceManager::LoadTexture(
                UserInterface->FileDialog->GetSelected().string().c_str(),
                Util::IsPng(file_name),
                file_name);
        this->ActivePage->BeginPlacePiece(
                new GameObject(
                        glm::vec2(0.0f, 0.0f),
                        glm::vec2(98.0f, 98.0f),
                        ResourceManager::GetTexture(file_name)));
        UserInterface->FileDialog->ClearSelected();
    }
    if (UserInterface->AddPage) {
        Page *new_page = this->MakePage(UserInterface->PageName);
        this->Pages.push_back(new_page);
        this->ActivePage = new_page;
        UserInterface->ActivePage = this->Pages.size() - 1;
    }
    UserInterface->ClearFlags();
}

Page *Game::MakePage(std::string name) {
    SpriteRenderer *BoardRenderer =
            new SpriteRenderer(ResourceManager::GetShader("sprite"), 20);
    return new Page(
            name,
            ResourceManager::GetTexture("grid"),
            BoardRenderer,
            glm::vec2(0.0f, 0.0f),
            glm::vec2(20.0f, 20.0f));
}