#include "camera.h"
#include "glfw_handler.h"

static const float DELTA_ZOOM = 1.05f;

Camera2D::Camera2D(
        float bounds, glm::vec2 zoom_bounds) : Bounds(bounds),
        ZoomBounds(zoom_bounds) {
}

void Camera2D::Move(glm::vec2 delta_mv) {
    this->Position -= delta_mv;
}

void Camera2D::Zoom(glm::ivec2 mouse_pos, int direction) {
    glm::vec2 mouse_to_board = -this->Position - (glm::vec2)mouse_pos;
    if (direction == 1) {
        if (this->ZoomFactor <= this->ZoomBounds.y) {
            this->ZoomFactor *= DELTA_ZOOM;
            mouse_to_board *= DELTA_ZOOM;
        }
    } else if (direction == -1) {
        if (this->ZoomFactor >= this->ZoomBounds.x) {
            this->ZoomFactor /= DELTA_ZOOM;
            mouse_to_board /= DELTA_ZOOM;
        }
    }
    this->Position = -(glm::vec2)mouse_pos - mouse_to_board;
}

glm::mat4 Camera2D::CalculateView(glm::vec2 board_dims) {
    static GLFW &glfw = GLFW::GetInstance();

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
    // Translate on camera position
    view = glm::translate(view, glm::vec3(-this->Position, 0.0f));
    // Scale based on camera zoom
    view = glm::scale(
            view, glm::vec3(this->ZoomFactor, this->ZoomFactor, 1.0f));
    return view;
}