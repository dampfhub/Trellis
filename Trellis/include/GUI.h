#ifndef GUI_H
#define GUI_H

#include "imgui.h"

class GUI {
public:
    GUI(GUI const &) = delete; // Disallow copying
    void operator=(GUI const &) = delete;

    static GUI &GetInstance();

    void NewFrame();

    void Render();

    void SetCursor(ImGuiMouseCursor_ cursor);
private:
    // Declaration of io must appear above Want... members for correct
    // constructor initialization order.
    ImGuiIO &io;
public:
    const bool &WantCaptureMouse;
    const bool &WantCaptureKeyboard;
    const bool &WantTextInput;
    const bool &WantSetMousePos;
    const bool &WantSaveIniSettings;

private:
    GUI();

    ~GUI();
};

#endif //GUI_H
