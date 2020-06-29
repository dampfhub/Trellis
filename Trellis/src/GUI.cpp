#include "GUI.h"

#include "glfw_handler.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

GUI &
GUI::GetInstance() {
    static GUI instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

GUI::GUI()
    : io((ImGui::CreateContext(), ImGui::GetIO()))
    , WantCaptureMouse(io.WantCaptureMouse)
    , WantCaptureKeyboard(io.WantCaptureKeyboard)
    , WantTextInput(io.WantTextInput)
    , WantSetMousePos(io.WantSetMousePos)
    , WantSaveIniSettings(io.WantSaveIniSettings) {
    IMGUI_CHECKVERSION();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Specific styling
    ImGui::GetStyle().WindowRounding   = 0.0f;
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
    ImGui::GetStyle().WindowBorderSize = 0.0f;

    GLFW &glfw = GLFW::GetInstance();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(glfw.GetWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    DefaultFont = io.Fonts->AddFontFromFileTTF("fonts/Roboto.TTF", 20.0f);
    MediumFont = io.Fonts->AddFontFromFileTTF("fonts/Roboto.TTF", 40.0f);
    BigFont = io.Fonts->AddFontFromFileTTF("fonts/Roboto.TTF", 80.0f);
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void
GUI::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void
GUI::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
GUI::SetCursor(ImGuiMouseCursor_ cursor) {
    ImGui::SetMouseCursor(cursor);
}
