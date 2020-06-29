#include "GUI.h"
#include "glfw_handler.h"

#include <array>
#include <functional>

using std::function, std::array;

static array<function<keyfunc>, GLFW_KEY_LAST + 1>            key_press_callbacks{nullptr};
static array<function<keyfunc>, GLFW_KEY_LAST + 1>            key_release_callbacks{nullptr};
static array<function<mousefunc>, GLFW_MOUSE_BUTTON_LAST + 1> mouse_press_callbacks{nullptr};
static array<function<mousefunc>, GLFW_MOUSE_BUTTON_LAST + 1> mouse_release_callbacks{nullptr};

static function<scrollfunc>    scroll_callback      = nullptr;
static function<windowsizefun> window_size_callback = nullptr;
static function<mouseposfunc>  mouse_pos_callback   = nullptr;

// The width of the screen
static int screen_width = 1000;
// The height of the screen
static int screen_height = 800;

GLFW &
GLFW::GetInstance() {
    static GLFW instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

static void
window_size_handler(GLFWwindow *window, int width, int height) {
    (void)window;
    screen_width  = width;
    screen_height = height;
    if (window_size_callback) { window_size_callback(width, height); }
}

static void
key_handler(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)window;
    static GUI &gui = GUI::GetInstance();
    if (action == GLFW_PRESS) {
        if (gui.WantCaptureKeyboard) { return; }
        if (key_press_callbacks[key] != nullptr) {
            key_press_callbacks[key](key, scancode, action, mods);
        }
    } else if (action == GLFW_RELEASE) {
        if (key_release_callbacks[key] != nullptr) {
            key_release_callbacks[key](key, scancode, action, mods);
        }
    }
}

static void
mouse_handler(GLFWwindow *window, int button, int action, int mods) {
    (void)window;
    static GUI &gui = GUI::GetInstance();
    if (action == GLFW_PRESS) {
        if (gui.WantCaptureMouse) { return; }
        if (mouse_press_callbacks[button] != nullptr) {
            mouse_press_callbacks[button](button, action, mods);
        }
    } else if (action == GLFW_RELEASE) {
        if (mouse_release_callbacks[button] != nullptr) {
            mouse_release_callbacks[button](button, action, mods);
        }
    }
}

static void
scroll_handler(GLFWwindow *window, double xoffset, double yoffset) {
    (void)window;
    static GUI &gui = GUI::GetInstance();
    if (gui.WantCaptureMouse) { return; }
    if (scroll_callback != nullptr) { scroll_callback(xoffset, yoffset); }
}

static void
mouse_pos_handler(GLFWwindow *window, double x, double y) {
    (void)window;
    if (mouse_pos_callback != nullptr) { mouse_pos_callback(x, y); }
}

static void
framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    (void)window;
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

GLFW::GLFW() {
    glfwInit();
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, true);

    window = glfwCreateWindow(screen_width, screen_height, "Trellis", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_handler);
    glfwSetMouseButtonCallback(window, mouse_handler);
    glfwSetScrollCallback(window, scroll_handler);
    glfwSetCursorPosCallback(window, mouse_pos_handler);
    glfwSetWindowSizeCallback(window, window_size_handler);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

GLFW::~GLFW() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

int
GLFW::WindowShouldClose() const {
    return glfwWindowShouldClose(window);
}

void
GLFW::SetWindowShouldClose(int value) {
    glfwSetWindowShouldClose(window, value);
}

void
GLFW::SwapBuffers() {
    glfwSwapBuffers(window);
}

void
GLFW::SetWindowIcon(unsigned char *pixels, int width, int height) {
    GLFWimage icon;
    icon.pixels = pixels;
    icon.width  = width;
    icon.height = height;
    glfwSetWindowIcon(window, 1, &icon);
}

GLFWwindow *
GLFW::GetWindow() {
    return window;
}

void
GLFW::RegisterKey(int key, const function<keyfunc> &callback) {
    if (key < 0 || key > GLFW_KEY_LAST) { return; }
    key_press_callbacks[key]   = callback;
    key_release_callbacks[key] = callback;
}

void
GLFW::RegisterKeyPress(int key, const function<keyfunc> &callback) {
    if (key < 0 || key > GLFW_KEY_LAST) { return; }
    key_press_callbacks[key] = callback;
}

void
GLFW::RegisterKeyRelease(int key, const function<keyfunc> &callback) {
    if (key < 0 || key > GLFW_KEY_LAST) { return; }
    key_release_callbacks[key] = callback;
}

void
GLFW::RegisterMouse(int button, const function<mousefunc> &callback) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) { return; }
    mouse_press_callbacks[button]   = callback;
    mouse_release_callbacks[button] = callback;
}

void
GLFW::RegisterMousePress(int button, const function<mousefunc> &callback) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) { return; }
    mouse_press_callbacks[button] = callback;
}

void
GLFW::RegisterMouseRelease(int button, const function<mousefunc> &callback) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) { return; }
    mouse_release_callbacks[button] = callback;
}

void
GLFW::RegisterScroll(const function<scrollfunc> &callback) {
    scroll_callback = callback;
}

void
GLFW::RegisterMousePosCallback(const function<mouseposfunc> &callback) {
    mouse_pos_callback = callback;
}

void
GLFW::RegisterWindowSizeCallback(const function<windowsizefun> &callback) {
    window_size_callback = callback;
}

int
GLFW::GetScreenWidth() {
    return screen_width;
}

int
GLFW::GetScreenHeight() {
    return screen_height;
}
void
GLFW::UnregisterKey(int key) {
    key_press_callbacks[key]   = nullptr;
    key_release_callbacks[key] = nullptr;
}
void
GLFW::UnregisterKeyPress(int key) {
    key_press_callbacks[key]   = nullptr;
}
void
GLFW::UnregisterKeyRelease(int key) {
    key_release_callbacks[key]   = nullptr;
}
void
GLFW::UnregisterMouse(int button) {
    mouse_press_callbacks[button] = nullptr;
    mouse_release_callbacks[button] = nullptr;
}
void
GLFW::UnregisterMousePress(int button) {
    mouse_press_callbacks[button] = nullptr;
}
void
GLFW::UnregisterMouseRelease(int button) {
    mouse_release_callbacks[button] = nullptr;
}
void
GLFW::UnregisterScroll() {
    scroll_callback = nullptr;
}
void
GLFW::UnregisterMousePosCallback() {
    mouse_pos_callback = nullptr;
}
void
GLFW::UnregisterWindowSizeCallback() {
    window_size_callback = nullptr;
}
