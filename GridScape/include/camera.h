#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>

class Camera2D {
public:
    glm::vec2 Position;
    float ZoomFactor;

    float Bounds;
    glm::vec2 ZoomBounds;

    glm::ivec2 MousePos = glm::ivec2(0);

    Camera2D(
            float bounds, glm::vec2 board_dimensions, glm::vec2 zoom_bounds);

    void Move(glm::vec2 delta_mv);

    void Zoom(glm::ivec2 pos, int direction);

    glm::mat4 CalculateView(glm::vec2 board_dims);
};

#endif