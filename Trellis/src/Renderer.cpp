#include "renderer.h"

#include "glfw_handler.h"
#include "glm/glm.hpp"
#include "resource_manager.h"

using std::shared_ptr;

Renderer::Renderer(
  const shared_ptr<Shader> &shader,
  const Transform &         transform,
  const glm::mat4 &         view)
    : shader(shader)
    , transform(transform)
    , view(view) {}

glm::mat4
Renderer::Model() {
    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(
      model,
      glm::vec3(
        transform.position,
        0.0f)); // first translate (transformations are: scale happens first, then rotation, and
                          // then final translation happens; reversed order)

    model = glm::translate(
      model,
      glm::vec3(
        0.5f * transform.scale.x,
        0.5f * transform.scale.y,
        0.0f)); // move origin of rotation to center of quad
    model = glm::rotate(
      model,
      glm::radians(transform.rotation),
      glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    model = glm::translate(
      model,
      glm::vec3(
        -0.5f * transform.scale.x,
        -0.5f * transform.scale.y,
        0.0f)); // move origin back

    model = glm::scale(model, glm::vec3(transform.scale, 1.0f)); // last scale
    return model;
}

Renderer::~Renderer() {}
