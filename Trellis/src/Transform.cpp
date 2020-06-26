#include "transform.h"

Transform::Transform() : position(0, 0),
        scale(1, 1),
        rotation(0) {

}

Transform::Transform(
        const glm::vec2 &position,
        const glm::vec2 &scale,
        float rotation) : position(position),
        scale(scale),
        rotation(rotation) {
}
