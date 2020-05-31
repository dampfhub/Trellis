#ifndef GUI_H
#define GUI_H

class GUI {
public:
    GUI(GUI const&) = delete; // Disallow copying
    void operator=(GUI const&) = delete;
    static GUI &getInstance();

    void NewFrame();
    void Render();

private:
    GUI();
    ~GUI();
};

#endif //GUI_H
