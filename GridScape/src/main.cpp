#include "game.h"
#include "glfw_handler.h"
#include "resource_manager.h"
#include "stb_image.h"
#include "gui.h"

#include <iostream>

int main(int argc, char * argv[]) {
    GLFW &glfw = GLFW::getInstance();

	// glad: load all OpenGL function pointersmode->height
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    GUI &gui = GUI::getInstance();
    Game &Dnd = Game::getInstance();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set window icon
	int width, height;
	unsigned char *icon = stbi_load("textures/orcling.png", &width, &height, 0, 4);
	glfw.SetWindowIcon(icon, width, height);
	stbi_image_free(icon);

	// deltaTime variables
	// -------------------

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// start game within menu state
	// ----------------------------
	Dnd.State = GAME_MENU;

	while (!glfw.WindowShouldClose()) {
		// calculate delta time
		// --------------------
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		// Start the Dear ImGui frame
		gui.NewFrame();

		// update game state
		// -----------------
		Dnd.Update(deltaTime);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		Dnd.Render();
		gui.Render();
		glfw.SwapBuffers();
	}

	// delete all resources as loaded using the resource manager
	// ---------------------------------------------------------
	return 0;
}
