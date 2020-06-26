#include "board_renderer.h"
#include "resource_manager.h"
#include "glfw_handler.h"

BoardRenderer::BoardRenderer(
        const Transform &transform, const glm::mat4 &view) : Renderer(
        ResourceManager::GetShader("board"), transform, view) {
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

void BoardRenderer::Draw() {
    shader->Use();
    glm::mat4 model = Model();
    shader->SetMatrix4("view", view);
    shader->SetMatrix4("model", model);
    shader->SetVector2f(
            "screenRes",
            glm::vec2(GLFW::GetScreenWidth(), GLFW::GetScreenHeight()));

    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

BoardRenderer::~BoardRenderer() {
    glDeleteVertexArrays(1, &quad_VAO);
}
