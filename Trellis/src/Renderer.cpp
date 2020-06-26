#include "renderer.h"
#include "resource_manager.h"
#include "glm/glm.hpp"
#include "glfw_handler.h"

using std::shared_ptr;

Renderer::Renderer(
        const shared_ptr<Shader> &shader,
        const Transform &transform,
        const glm::mat4 &view) : shader(
        shader),
        transform(transform),
        view(view) {
}

glm::mat4 Renderer::Model() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(
            model, glm::vec3(
                    transform.position,
                    0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    model = glm::translate(
            model, glm::vec3(
                    0.5f * transform.scale.x,
                    0.5f * transform.scale.y,
                    0.0f)); // move origin of rotation to center of quad
    model = glm::rotate(
            model,
            glm::radians(transform.rotation),
            glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    model = glm::translate(
            model, glm::vec3(
                    -0.5f * transform.scale.x,
                    -0.5f * transform.scale.y,
                    0.0f)); // move origin back

    model = glm::scale(model, glm::vec3(transform.scale, 1.0f)); // last scale
    return model;
}

Renderer::~Renderer() {

}

SRenderer::SRenderer(
        const Transform &transform,
        const glm::mat4 &view,
        const Texture2D &sprite) : Renderer(
        ResourceManager::GetShader("sprite"), transform, view),
        Sprite(sprite) {
    unsigned int VBO;
    float vertices[] = {
            // pos      // tex
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            0.0f };
    glGenVertexArrays(1, &quad_VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(quad_VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
            0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SRenderer::Draw() {
    shader->Use();
    glm::mat4 model = Model();
    shader->SetMatrix4("view", view);
    shader->SetMatrix4("model", model);
    shader->SetVector2f(
            "screenRes",
            glm::vec2(GLFW::GetScreenWidth(), GLFW::GetScreenHeight()));
    glActiveTexture(GL_TEXTURE0);
    Sprite.Bind();

    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

SRenderer::~SRenderer() {
    glDeleteVertexArrays(1, &quad_VAO);
}
