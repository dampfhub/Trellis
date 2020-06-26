#include "sprite_renderer.h"
#include "resource_manager.h"
#include "glfw_handler.h"

SpriteRenderer::SpriteRenderer(const Shader &shader, int tile_factor): sprite_shader(shader) {
    init_render_data(tile_factor);
}

SpriteRenderer::~SpriteRenderer() {
    glDeleteVertexArrays(1, &quad_VAO);
}

void SpriteRenderer::DrawSprite(
        Texture2D texture,
        glm::vec2 position,
        int border_pixel_width,
        glm::vec2 size,
        float rotate,
        glm::vec3 color) {
    static GLFW &glfw = GLFW::GetInstance();
    // prepare transformations
    sprite_shader.Use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(
            model, glm::vec3(
                    position,
                    0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    model = glm::translate(
            model, glm::vec3(
                    0.5f * size.x,
                    0.5f * size.y,
                    0.0f)); // move origin of rotation to center of quad
    model = glm::rotate(
            model,
            glm::radians(rotate),
            glm::vec3(0.0f, 0.0f, 1.0f)); // then rotate
    model = glm::translate(
            model, glm::vec3(
                    -0.5f * size.x, -0.5f * size.y, 0.0f)); // move origin back

    model = glm::scale(model, glm::vec3(size, 1.0f)); // last scale

    sprite_shader.SetMatrix4("view", View);
    sprite_shader.SetMatrix4("model", model);
    sprite_shader.SetVector2f(
            "screenRes",
            glm::vec2(glfw.GetScreenWidth(), glfw.GetScreenHeight()));

    // render textured quad
    sprite_shader.SetVector3f("spriteColor", color);

    sprite_shader.SetFloat("aspect", 1.0f);
    sprite_shader.SetInteger("border_width", border_pixel_width);

    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpriteRenderer::Resize(int size) {
    init_render_data(size);
}

void SpriteRenderer::init_render_data(int tile_factor) {
    // configure VAO/VBO
    unsigned int VBO;
    float tex_coord = 1.0f * tile_factor;
    float vertices[] = {
            // pos      // tex
            0.0f,
            1.0f,
            0.0f,
            tex_coord,
            1.0f,
            0.0f,
            tex_coord,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            1.0f,
            0.0f,
            tex_coord,
            1.0f,
            1.0f,
            tex_coord,
            tex_coord,
            1.0f,
            0.0f,
            tex_coord,
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
