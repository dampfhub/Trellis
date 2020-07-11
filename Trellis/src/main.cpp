#include "glfw_handler.h"
#include "gui.h"
#include "stb_image.h"
#include "state_manager.h"
#include "network_manager.h"

#include <iostream>

int
main() {
    GLFW &glfw = GLFW::GetInstance();

    // glad: load all OpenGL function pointersmode->height
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GUI &gui = GUI::GetInstance();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set window icon
    int            width, height;
    unsigned char *icon = stbi_load("textures/orcling.png", &width, &height, 0, 4);
    glfw.SetWindowIcon(icon, width, height);
    stbi_image_free(icon);

    // deltaTime variables
    // -------------------

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    StateManager &  sm = StateManager::GetInstance();
    NetworkManager &nm = NetworkManager::GetInstance();

    while (!glfw.WindowShouldClose()) {
        // calculate delta time
        // --------------------
        auto currentFrame = (float)glfwGetTime();
        deltaTime         = currentFrame - lastFrame;
        lastFrame         = currentFrame;
        glfwPollEvents();

        // Start the Dear ImGui frame
        gui.NewFrame();

        // update network manager. currently this calls all the callbacks for http requests
        nm.Update();

        // update game state
        // -----------------
        sm.Update();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        sm.Draw();
        gui.Render();

        ImGuiIO &io = ImGui::GetIO();
        /*
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }*/
        glfw.SwapBuffers();
    }

    // delete all resources as loaded using the resource manager
    // ---------------------------------------------------------
    return 0;
}
