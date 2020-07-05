#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "data.h"
#include "shader.h"
#include "texture.h"

#include <glad/glad.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ResourceManager {
public:
    ResourceManager(ResourceManager const &) = delete; // Disallow copying
    void operator=(ResourceManager const &) = delete;

    static ResourceManager &GetInstance();

    // resource storage
    std::unordered_map<uint64_t, Data::ImageData> Images;

    // loads (and generates) a sprite_shader program from file loading vertex, fragment (and
    // geometry) sprite_shader's source code. If gShaderFile is not nullptr, it also loads a
    // geometry sprite_shader
    std::shared_ptr<Shader> LoadShader(
        const char *       vShaderFile,
        const char *       fShaderFile,
        const char *       gShaderFile,
        const std::string &name);

    // retrieves a stored sader
    std::shared_ptr<Shader> GetShader(const std::string &name);

    // loads (and generates) a texture from file
    std::shared_ptr<Texture2D> LoadTexture(const char *file);

    std::shared_ptr<Texture2D> GetTexture(uint64_t uid);

    void SetGlobalFloat(const char *name, float value);
    void SetGlobalInteger(const char *name, int value);
    void SetGlobalVector2f(const char *name, const glm::vec2 &value);
    void SetGlobalVector3f(const char *name, const glm::vec3 &value);
    void SetGlobalVector4f(const char *name, const glm::vec4 &value);
    void SetGlobalMatrix4(const char *name, const glm::mat4 &value);

    void WriteToDB(const SQLite::Database &db);
    void ReadFromDB(const SQLite::Database &db, uint64_t ImageUID);

private:
    // private constructor, that is we do not want any actual resource manager objects. Its members
    // and functions should be publicly available (static).
    ResourceManager() {}

    ~ResourceManager();

    // loads and generates a sprite_shader from file
    std::shared_ptr<Shader> loadShaderFromFile(
        const char *vShaderFile,
        const char *fShaderFile,
        const char *gShaderFile = nullptr);

    // loads a single texture from file
    std::shared_ptr<Texture2D> loadTextureFromFile(const char *file);

    // loads a single texture from a cached texture uid
    std::shared_ptr<Texture2D> loadTextureFromUID(uint64_t uid);

    std::unordered_map<std::string, std::shared_ptr<Shader>> Shaders;
    std::unordered_map<uint64_t, std::shared_ptr<Texture2D>> Textures;
};

#endif
