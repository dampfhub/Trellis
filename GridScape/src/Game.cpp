#include <iostream>

#include "game.h"
#include "sprite_renderer.h"
#include "resource_manager.h"
#include "page.h"
#include "game_object.h"
#include "ui.h"
#include "util.h"

SpriteRenderer * ObjectRenderer;
UI * UserInterface;

Game & Game::getInstance() {
    static Game instance; // Guaranteed to be destroyed.
                          // Instantiated on first use.
    return instance;
}

Game::~Game() {
	for (Page * page : this->Pages) {
		delete page;
	}
	delete ObjectRenderer;
	delete UserInterface;
}

void Game::Init(int width, int height) {
    this->ScreenDims = std::make_shared<std::pair<int, int>>(width, height);
	this->init_shaders();
	this->init_textures();
	this->init_objects();

	// Set projection matrix
	this->set_projection();
	glm::mat4 view = glm::mat4(1.0f);
	ResourceManager::GetShader("sprite").SetMatrix4("view", view, true);
}

void Game::SetScreenDims(int width, int height) {
    this->ScreenDims->first = width;
    this->ScreenDims->second = height;
    this->set_projection();
}

void Game::init_shaders() {
	ResourceManager::LoadShader("shaders/sprite.vert", "shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::GetShader("sprite").SetInteger("image", 0, true);
}

void Game::init_textures() {
	ResourceManager::LoadTexture("textures/grid.png", false, "grid");
	ResourceManager::LoadTexture("textures/token.png", false, "goblin");
	ResourceManager::LoadTexture("textures/orcling.png", true, "orcling");
}

void Game::init_objects() {
	ObjectRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
	SpriteRenderer * BoardRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"), 20);
	UserInterface = new UI(this->ScreenDims);
	this->ActivePage = new Page("Default", ResourceManager::GetTexture("grid"), BoardRenderer,
								this->ScreenDims,
								glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));
	this->Pages.push_back(this->ActivePage);
	GameObject * test = new GameObject(glm::vec2(1.0f, 1.0f), glm::vec2(98.0f, 98.0f),
									   ResourceManager::GetTexture("goblin"));
	GameObject * orc = new GameObject(glm::vec2(2.0f, 1.0f), glm::vec2(98.0f, 98.0f),
									  ResourceManager::GetTexture("orcling"));
	this->ActivePage->PlacePiece(test);
	this->ActivePage->PlacePiece(orc);
}

void Game::set_projection() {
    glm::mat4 projection = glm::ortho(0.0f,
            static_cast<float>(this->ScreenDims->first),
            static_cast<float>(this->ScreenDims->second),
            0.0f,
            -1.0f,
            1.0f);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection, true);
}

void Game::Update(float dt) {
	this->ProcessUIEvents();
	this->ActivePage->Update(dt);
	if (this->ActivePage->Placing)
		this->ActivePage->UpdatePlacing(this->MousePos);
}

void Game::ProcessInput(float dt) {
	if (this->Keys[GLFW_KEY_DELETE]) {
	}
}

void Game::ProcessMouse(float dt) {
	if (this->LeftClickPress) {
		this->ActivePage->HandleLeftClickPress(this->MousePos);
		this->LeftClickPress = false;
	} else if (this->LeftClickHold) {
		this->ActivePage->HandleLeftClickHold(this->MousePos);
	} else if (this->LeftClickRelease) {
		this->ActivePage->HandleLeftClickRelease(this->MousePos);
		this->LeftClickRelease = false;
	}
	if (this->RightClick) {
		this->ActivePage->HandleRightClick(this->MousePos);
		this->RightClick = false;
	}
	if (this->MiddleClickPress) {
		this->ActivePage->HandleMiddleClickPress(this->MousePos);
		this->MiddleClickPress = false;
	} else if (this->MiddleClickHold) {
		this->ActivePage->HandleMiddleClickHold(this->MousePos);
	}
	if (this->ScrollDirection != 0) {
		this->ActivePage->HandleScrollWheel(this->ScrollDirection);
		this->ScrollDirection = 0;
	}
}

void Game::Render() {
	this->ActivePage->Draw(ObjectRenderer, nullptr);
	UserInterface->Draw(this->Pages, this->ActivePage);
}

void Game::ProcessUIEvents() {
	this->ActivePage = UserInterface->GetActivePage(this->Pages);
	if (UserInterface->FileDialog->HasSelected()) {
		std::string file_name = Util::PathBaseName(UserInterface->FileDialog->GetSelected().string());
		ResourceManager::LoadTexture(UserInterface->FileDialog->GetSelected().string().c_str(),
									 Util::IsPng(file_name), file_name);
		this->ActivePage->BeginPlacePiece(
			new GameObject(glm::vec2(0.0f, 0.0f), glm::vec2(98.0f, 98.0f),
						   ResourceManager::GetTexture(file_name)));
		UserInterface->FileDialog->ClearSelected();
	}
	if (UserInterface->AddPage) {
		Page * new_page = this->MakePage(UserInterface->PageName);
		this->Pages.push_back(new_page);
		this->ActivePage = new_page;
		UserInterface->ActivePage = this->Pages.size() - 1;
	}
	UserInterface->ClearFlags();
}

Page * Game::MakePage(std::string name) {
	SpriteRenderer * BoardRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"), 20);
	return new Page(name, ResourceManager::GetTexture("grid"), BoardRenderer,
					this->ScreenDims,
					glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 20.0f));
}