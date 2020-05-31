#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

#include "page.h"

// Represents the current state of the game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {
public:
	// game state
	GameState               State;
	bool                    Keys[1024];
	bool                    LeftClickPress = false;
	bool LeftClickHold = false;
	bool LeftClickRelease = false;
	bool RightClick = false;
	bool MiddleClickPress = false;
	bool MiddleClickHold = false;
	glm::ivec2              MousePos;
	int                     ScrollDirection;

	Page * ActivePage;
	std::vector<Page *>      Pages;

	// constructor/destructor
	Game(unsigned int width, unsigned int height);
	~Game();
	// initialize game state (load all shaders/textures/levels)
	void Init();
	// setters
	void SetScreenDims(int width, int height);
	// game loop
	void ProcessInput(float dt);
	void ProcessMouse(float dt);
	void Update(float dt);
	void Render();

private:
    std::shared_ptr<std::pair<int, int>> ScreenDims;

	void init_textures();
	void init_shaders();
	void init_objects();

	void set_projection();

	void ProcessUIEvents();

	Page * MakePage(std::string name);
};

#endif