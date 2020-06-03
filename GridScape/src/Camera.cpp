#include <iostream>

#include "camera.h"
#include "glfw_handler.h"
#include <iostream>

Camera2D::Camera2D(
        float bounds,
        glm::vec2 board_dimensions,
        glm::vec2 zoom_bounds) : Bounds(bounds),
        ZoomBounds(zoom_bounds),
        Position(glm::vec2(0.0f, 0.0f)),
        ZoomFactor(1.0f) {
}

void Camera2D::Move(glm::vec2 delta_mv) {
    this->Position -= delta_mv;
}

void Camera2D::Zoom(glm::ivec2 pos, int direction) {
    if (direction == 1) {
        this->ZoomFactor += 0.05f;
        this->Position += (glm::vec2)pos * 0.05f;
    } else if (direction == -1) {
        this->ZoomFactor -= 0.05f;
        this->Position -= (glm::vec2)pos * 0.05f;
    }
    this->MousePos = pos;
}

glm::mat4 Camera2D::CalculateView(glm::vec2 board_dims) {
    static GLFW &glfw = GLFW::GetInstance();

    // Check if would zoom out of zoom bounds
    if (this->ZoomFactor < this->ZoomBounds.x) {
        this->ZoomFactor = this->ZoomBounds.x;
    }
    if (this->ZoomFactor > this->ZoomBounds.y) {
        this->ZoomFactor = this->ZoomBounds.y;
    }

    // Scuffed right boundary equation (<3 Taylor)
    // Comes from:
    // Solve[m == (d - p)*z, p] // FullSimplify
    // { { p->d - m / z } }
    if ((board_dims.x * this->ZoomFactor - this->Position.x) <
            ((float)glfw.GetScreenWidth() - this->Bounds)) {
        this->Position.x = board_dims.x * this->ZoomFactor -
                ((float)glfw.GetScreenWidth() - this->Bounds);
    }
    if ((board_dims.y * this->ZoomFactor - this->Position.y) <
            ((float)glfw.GetScreenHeight() - this->Bounds)) {
        this->Position.y = board_dims.y * this->ZoomFactor -
                ((float)glfw.GetScreenHeight() - this->Bounds);
    }
    // Check if camera would move out of bounds
    if (this->Position.x < -this->Bounds) {
        this->Position.x = -this->Bounds;
    }
    if (this->Position.y < -this->Bounds) {
        this->Position.y = -this->Bounds;
    }

    glm::mat4 view = glm::mat4(1.0f);
    glm::vec4 mouse_screen = glm::vec4(this->MousePos, 0.0f, 1.0f);
    // Translate on camera position
    view = glm::translate(view, glm::vec3(-this->Position, 0.0f));
    // Scale based on camera zoom
    view = glm::scale(
            view, glm::vec3(this->ZoomFactor, this->ZoomFactor, 1.0f));
    return view;
}