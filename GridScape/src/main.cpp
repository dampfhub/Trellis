#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "resource_manager.h"
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

// GLFW function declerations
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void window_size_callback(GLFWwindow * window, int width, int height);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);

// The Width of the screen
const unsigned int SCREEN_WIDTH = 1600;
// The height of the screen
const unsigned int SCREEN_HEIGHT = 1000;

Game Dnd(0, 0);

int main(int argc, char * argv[]) {
	glfwInit();
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, true);

	//GLFWwindow * window = glfwCreateWindow(mode->width, mode->height, "Dnd", glfwGetPrimaryMonitor(), nullptr);
	GLFWwindow * window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dnd", nullptr, nullptr);
	//Dnd = Game(mode->width, mode->height);
	Dnd = Game(SCREEN_WIDTH, SCREEN_HEIGHT);
	glfwMakeContextCurrent(window);

	// glad: load all OpenGL function pointersmode->height
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Specific styling
	ImGui::GetStyle().WindowRounding = 0.0f;
	ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
	ImGui::GetStyle().WindowBorderSize = 0.0f;

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	io.Fonts->AddFontFromFileTTF("fonts/Roboto.TTF", 20.0f);

	// Setup input callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	// OpenGL configuration
	// --------------------
	//glViewport(0, 0, mode->width, mode->height);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// initialize game
	// ---------------
	Dnd.Init();

	// Set window icon 
	GLFWimage icons[1];
	icons[0].pixels = stbi_load("textures/orcling.png", &icons[0].width, &icons[0].height, 0, 4);
	glfwSetWindowIcon(window, 1, icons);

	// deltaTime variables
	// -------------------

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// start game within menu state
	// ----------------------------
	Dnd.State = GAME_MENU;

	while (!glfwWindowShouldClose(window)) {
		// calculate delta time
		// --------------------
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// manage user input
		// -----------------
		if (!io.WantCaptureKeyboard)
			Dnd.ProcessInput(deltaTime);
		if (!io.WantCaptureMouse)
			Dnd.ProcessMouse(deltaTime);

		// update game state
		// -----------------
		Dnd.Update(deltaTime);

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		Dnd.Render();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// delete all resources as loaded using the resource manager
	// ---------------------------------------------------------
	ResourceManager::Clear();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode) {
	ImGuiIO & io = ImGui::GetIO();
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;
	if (io.WantCaptureKeyboard) {
		if (action == GLFW_PRESS)
			io.KeysDown[key] = true;
		return;
	}
	// when a user presses the escape key, we set the WindowShouldClose property to true, closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			Dnd.Keys[key] = true;
		else if (action == GLFW_RELEASE)
			Dnd.Keys[key] = false;
	}
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
	ImGuiIO & io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		if (action == GLFW_PRESS) {
			io.MouseClicked[button] = true;
			return;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			Dnd.LeftClickPress = true;
			Dnd.LeftClickHold = true;
		} else {
			Dnd.LeftClickHold = false;
			Dnd.LeftClickRelease = true;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		Dnd.RightClick = true;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		if (action == GLFW_PRESS) {
			Dnd.MiddleClickPress = true;
			Dnd.MiddleClickHold = true;
		} else if (action == GLFW_RELEASE) {
			Dnd.MiddleClickHold = false;
		}
	}
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	Dnd.MousePos = glm::ivec2((int)x, (int)y);
}

void cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
	ImGuiIO & io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		io.MousePos = ImVec2(xpos, ypos);
		return;
	}
	Dnd.MousePos = glm::ivec2((int)xpos, (int)ypos);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset) {
	ImGuiIO & io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		io.MouseWheelH += (float)xoffset;
		io.MouseWheel += (float)yoffset;
		return;
	}
	Dnd.ScrollDirection = (int)yoffset;
}

void window_size_callback(GLFWwindow * window, int width, int height) {
    Dnd.SetScreenDims(width, height);
}
