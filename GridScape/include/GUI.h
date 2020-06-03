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

    // Returns 1 if mouse was captured, 0 otherwise
    int MousePress(int button);

    void MouseRelease(int button);

    // Returns 1 if key was captured, 0 otherwise
    int KeyPress(int key);

    void KeyRelease(int key);

    void SetCursor(ImGuiMouseCursor_ cursor);

private:
    GUI();

    ~GUI();
};

#endif //GUI_H
