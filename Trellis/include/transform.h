#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>

class Transform {
    public:
    glm::vec2 position;
    glm::vec2 scale;
    float     rotation;

    Transform();
    Transform(const glm::vec2 &position, const glm::vec2 &scale, float rotation);
};

#endif // TRANSFORM_H
