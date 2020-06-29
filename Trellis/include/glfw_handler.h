#ifndef GLFW_HANDLER_H
#define GLFW_HANDLER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>

typedef void(keyfunc)(int key, int scancode, int action, int mods);

typedef void(mousefunc)(int button, int action, int mods);

typedef void(scrollfunc)(double xoffset, double yoffset);

typedef void(mouseposfunc)(double x, double y);

typedef void(windowsizefun)(int width, int height);

class GLFW {
public:
    GLFW(GLFW const &) = delete; // Disallow copying
    void operator=(GLFW const &) = delete;

    static GLFW &GetInstance();

    int WindowShouldClose() const;

    void SetWindowShouldClose(int value);

    void SwapBuffers();

    void SetWindowIcon(unsigned char *pixels, int width, int height);

    GLFWwindow *GetWindow();

    void RegisterKey(int key, const std::function<keyfunc> &callback);

    void UnregisterKey(int key);

    void RegisterKeyPress(int key, const std::function<keyfunc> &callback);

    void UnregisterKeyPress(int key);

    void RegisterKeyRelease(int key, const std::function<keyfunc> &callback);

    void UnregisterKeyRelease(int key);

    void RegisterMouse(int button, const std::function<mousefunc> &callback);

    void UnregisterMouse(int button);

    void RegisterMousePress(int button, const std::function<mousefunc> &callback);

    void UnregisterMousePress(int button);

    void RegisterMouseRelease(int button, const std::function<mousefunc> &callback);

    void UnregisterMouseRelease(int button);

    void RegisterScroll(const std::function<scrollfunc> &callback);

    void UnregisterScroll();

    void RegisterMousePosCallback(const std::function<mouseposfunc> &callback);

    void UnregisterMousePosCallback();

    void RegisterWindowSizeCallback(const std::function<windowsizefun> &callback);

    void UnregisterWindowSizeCallback();

    static int GetScreenWidth();

    static int GetScreenHeight();

private:
    GLFW();

    ~GLFW();

    GLFWwindow *window;
};

#endif // GLFW_HANDLER_H
