#ifndef GLFW_HANDLER_H
#define GLFW_HANDLER_H

#include <GLFW/glfw3.h>

typedef void (*keyfunc)(int key, int scancode, int action, int mods);
typedef void (*mousefunc)(int button, int action, int mods);
typedef void (*scrollfunc)(double xoffset, double yoffset);
typedef void (*mouseposfunc)(double x, double y);
typedef void (*windowsizefun)(int width, int height);

class GLFW {
public:
    GLFW(GLFW const&) = delete; // Disallow copying
    void operator=(GLFW const&) = delete;
    static GLFW &getInstance();

    int WindowShouldClose() const;
    void SetWindowShouldClose(int value);
    void SwapBuffers();
    void SetWindowIcon(unsigned char *pixels, int width, int height);
    GLFWwindow *GetWindow();

    void RegisterKey(int key, keyfunc callback);
    void RegisterKeyPress(int key, keyfunc callback);
    void RegisterKeyRelease(int key, keyfunc callback);

    void RegisterMouse(int button, mousefunc callback);
    void RegisterMousePress(int button, mousefunc callback);
    void RegisterMouseRelease(int button, mousefunc callback);

    void RegisterScroll(scrollfunc callback);

    void RegisterMousePosCallback(mouseposfunc callback);

    void RegisterWindowSizeCallback(windowsizefun callback);


    // The width of the screen
    int SCREEN_WIDTH = 1600;
    // The height of the screen
    int SCREEN_HEIGHT = 1000;

private:
    GLFW();
    ~GLFW();

    GLFWwindow *window;

};

#endif //GLFW_HANDLER_H
