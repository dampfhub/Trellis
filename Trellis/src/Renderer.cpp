#include "renderer.h"

Renderer::Renderer(
        const std::shared_ptr<Shader> &shader,
        Transform &transform) : shader(shader),
        transform(transform) {
}
