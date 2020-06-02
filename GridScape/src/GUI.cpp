#include "gui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glfw_handler.h"

GUI & GUI::getInstance() {
    static GUI instance; // Guaranteed to be destroyed.
                         // Instantiated on first use.
    return instance;
}

GUI::GUI() {
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

    GLFW &glfw = GLFW::getInstance();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(glfw.GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    io.Fonts->AddFontFromFileTTF("fonts/Roboto.TTF", 20.0f);
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int GUI::MousePress(int button) {
    ImGuiIO & io = ImGui::GetIO();
    io.MouseDown[button] = true;
    if (io.WantCaptureMouse) {
        return 1;
    }
    return 0;
}

void GUI::MouseRelease(int button) {
    ImGuiIO & io = ImGui::GetIO();
    io.MouseDown[button] = false;
}


int GUI::KeyPress(int key) {
    ImGuiIO & io = ImGui::GetIO();
    io.KeysDown[key] = true;
    if (io.WantCaptureKeyboard) {
        return 1;
    }
    return 0;
}

void GUI::KeyRelease(int key) {
    ImGuiIO & io = ImGui::GetIO();
    io.KeysDown[key] = false;
}

void GUI::SetCursor(ImGuiMouseCursor_ cursor) {
    ImGui::SetMouseCursor(cursor);
}
