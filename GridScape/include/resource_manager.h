#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>
#include <vector>

#include <glad/glad.h>

#include "texture.h"
#include "shader.h"

class ImageData {
public:
    bool Alpha;
    std::vector<unsigned char> Data;

    ImageData() = default;

    ImageData(bool alpha, std::vector<unsigned char> data) : Alpha(alpha),
            Data(data) {
    }
};

class ResourceManager {
public:
    ResourceManager(ResourceManager const &) = delete; // Disallow copying
    void operator=(ResourceManager const &) = delete;

    static ResourceManager &GetInstance();

    // resource storage
    static std::map<std::string, Shader> Shaders;
    static std::map<std::string, Texture2D> Textures;
    static std::map<uint64_t, ImageData> Images;

    // loads (and generates) a sprite_shader program from file loading vertex, fragment (and geometry) sprite_shader's source code. If gShaderFile is not nullptr, it also loads a geometry sprite_shader
    static Shader LoadShader(
            const char *vShaderFile,
            const char *fShaderFile,
            const char *gShaderFile,
            std::string name);

    // retrieves a stored sader
    static Shader GetShader(std::string name);

    // loads (and generates) a texture from file
    static Texture2D LoadTexture(
            const char *file, bool alpha, std::string name);

    // retrieves a stored texture
    static Texture2D GetTexture(std::string name);
    static Texture2D GetTexture(uint64_t uid);

private:
    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager() {
    }

    ~ResourceManager();

    // loads and generates a sprite_shader from file
    static Shader loadShaderFromFile(
            const char *vShaderFile,
            const char *fShaderFile,
            const char *gShaderFile = nullptr);

    // loads a single texture from file
    static Texture2D loadTextureFromFile(const char *file, bool alpha);

    // loads a single texture from a cached texture uid
    static Texture2D loadTextureFromUID(uint64_t uid);
};

#endif
