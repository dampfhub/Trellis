#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "stb_image.h"
#include "util.h"

// Instantiate static variables
std::map<std::string, Texture2D>    ResourceManager::Textures;
std::map<std::string, Shader>       ResourceManager::Shaders;
std::map<uint64_t, ImageData>       ResourceManager::Images;

ResourceManager &ResourceManager::GetInstance() {
    static ResourceManager instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

Shader ResourceManager::LoadShader(
        const char *vShaderFile,
        const char *fShaderFile,
        const char *gShaderFile,
        std::string name) {
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader ResourceManager::GetShader(std::string name) {
    return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(
        const char *file, bool alpha, std::string name) {
    Textures[name] = loadTextureFromFile(file, alpha);
    return Textures[name];
}

Texture2D ResourceManager::GetTexture(std::string name) {
    return Textures[name];
}

Texture2D ResourceManager::GetTexture(uint64_t uid) {
    return loadTextureFromUID(uid);
}

ResourceManager::~ResourceManager() {
    // (properly) delete all shaders
    for (auto iter : Shaders) {
        glDeleteProgram(iter.second.ID);
    }
    // (properly) delete all textures
    for (auto iter : Textures) {
        glDeleteTextures(1, &iter.second.ID);
    }
}

Shader ResourceManager::loadShaderFromFile(
        const char *vShaderFile,
        const char *fShaderFile,
        const char *gShaderFile) {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        // open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry sprite_shader path is present, also load a geometry sprite_shader
        if (gShaderFile != nullptr) {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::exception &e) {
        std::cout
                << "ERROR::SHADER: Failed to read sprite_shader files"
                << std::endl;
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    const char *gShaderCode = geometryCode.c_str();
    // 2. now create sprite_shader object from source code
    Shader shader;
    shader.Compile(
            vShaderCode,
            fShaderCode,
            gShaderFile != nullptr
            ? gShaderCode
            : nullptr);
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char *file, bool alpha) {
    // create texture object
    Texture2D texture;
    if (alpha) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // load image
    int width, height, nrChannels;
    //unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
    std::ifstream infile(file, std::ios_base::binary);
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(infile)),
            (std::istreambuf_iterator<char>()));
    unsigned char *data = stbi_load_from_memory(
            buffer.data(), buffer.size(), &width, &height, &nrChannels, 0);
    uint64_t uid = Util::generate_uid();
    // now generate texture
    texture.Generate(width, height, data, uid);
    Images[uid] = ImageData(alpha, buffer);
    stbi_image_free(data);
    return texture;
}

Texture2D ResourceManager::loadTextureFromUID(uint64_t uid) {
    Texture2D texture;
    ImageData d = Images[uid];
    if (d.Alpha) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    int width, height, nrChannels;
    unsigned char *data = stbi_load_from_memory(
            d.Data.data(), d.Data.size(), &width, &height, &nrChannels, 0);
    texture.Generate(width, height, data, uid);
    stbi_image_free(data);
    return texture;
}
