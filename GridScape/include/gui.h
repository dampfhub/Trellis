#ifndef GUI_H
#define GUI_H

#include "imgui.h"

class Gui {
public:
    Gui(Gui const &) = delete; // Disallow copying
    void operator=(Gui const &) = delete;

    static Gui &GetInstance();

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
    Gui();

    ~Gui();
};

#endif //GUI_H
