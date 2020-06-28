#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>

class Camera2D {
public:
    glm::vec2 Position   = glm::vec2(0.0f);
    float     ZoomFactor = 1.0f;

    float     Bounds;
    glm::vec2 ZoomBounds;

    Camera2D(float bounds, glm::vec2 zoom_bounds);

    void Move(glm::vec2 delta_mv);

    void Zoom(glm::ivec2 mouse_pos, int direction);

    glm::mat4 CalculateView(glm::vec2 board_dims);
};

#endif