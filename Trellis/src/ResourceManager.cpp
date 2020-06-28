#include "resource_manager.h"
#include "stb_image.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

using Data::ImageData;
using std::make_pair, std::make_shared, std::shared_ptr;

// Instantiate static variables
std::unordered_map<std::string, Texture2D>          ResourceManager::Textures;
std::unordered_map<std::string, shared_ptr<Shader>> ResourceManager::Shaders;
std::unordered_map<uint64_t, ImageData>             ResourceManager::Images;

ResourceManager &
ResourceManager::GetInstance() {
    static ResourceManager instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
}

std::shared_ptr<Shader>
ResourceManager::LoadShader(
    const char *vShaderFile,
    const char *fShaderFile,
    const char *gShaderFile,
    std::string name) {
    Shaders.insert(make_pair(name, loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile)));
    return Shaders.at(name);
}

std::shared_ptr<Shader>
ResourceManager::GetShader(std::string name) {
    return Shaders.at(name);
}

Texture2D
ResourceManager::LoadTexture(const char *file, bool alpha, std::string name) {
    Textures[name] = loadTextureFromFile(file, alpha);
    return Textures[name];
}

Texture2D
ResourceManager::GetTexture(std::string name) {
    return Textures[name];
}

Texture2D
ResourceManager::GetTexture(uint64_t uid) {
    return loadTextureFromUID(uid);
}

ResourceManager::~ResourceManager() {
    // (properly) delete all textures
    for (auto iter : Textures) { glDeleteTextures(1, &iter.second.ID); }
}

std::shared_ptr<Shader>
ResourceManager::loadShaderFromFile(
    const char *vShaderFile,
    const char *fShaderFile,
    const char *gShaderFile) {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try {
        // open files
        std::ifstream     vertexShaderFile(vShaderFile);
        std::ifstream     fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry sprite_shader path is present, also load a geometry sprite_shader
        if (gShaderFile != nullptr) {
            std::ifstream     geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::exception &e) {
        std::cout << "ERROR::SHADER: Failed to read sprite_shader files" << std::endl;
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    const char *gShaderCode = geometryCode.c_str();
    // 2. now create sprite_shader object from source code
    return make_shared<Shader>(
        vShaderCode,
        fShaderCode,
        gShaderFile != nullptr ? gShaderCode : nullptr);
}

Texture2D
ResourceManager::loadTextureFromFile(const char *file, bool alpha) {
    // create texture object
    Texture2D texture;
    if (alpha) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format    = GL_RGBA;
    }
    // load image
    int width, height, nrChannels;
    // unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
    std::ifstream              infile(file, std::ios_base::binary);
    std::vector<unsigned char> buffer(
        (std::istreambuf_iterator<char>(infile)),
        (std::istreambuf_iterator<char>()));
    for (auto &i : Images) {
        if (i.second.Hash == Util::hash_image(buffer)) { return loadTextureFromUID(i.first); }
    }
    unsigned char *data =
        stbi_load_from_memory(buffer.data(), buffer.size(), &width, &height, &nrChannels, 0);
    uint64_t uid = Util::generate_uid();
    // now generate texture
    texture.Generate(width, height, data, uid);
    Images[uid] = ImageData(alpha, buffer);
    stbi_image_free(data);
    return texture;
}

Texture2D
ResourceManager::loadTextureFromUID(uint64_t uid) {
    Texture2D texture;
    ImageData d = Images[uid];
    if (d.Alpha) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format    = GL_RGBA;
    }
    int            width, height, nrChannels;
    unsigned char *data =
        stbi_load_from_memory(d.Data.data(), d.Data.size(), &width, &height, &nrChannels, 0);
    texture.Generate(width, height, data, uid);
    stbi_image_free(data);
    return texture;
}

void
ResourceManager::SetGlobalFloat(const char *name, float value) {
    for (auto &[key, shader] : Shaders) { shader->SetFloat(name, value); }
}

void
ResourceManager::SetGlobalInteger(const char *name, int value) {
    for (auto &[key, shader] : Shaders) { shader->SetInteger(name, value); }
}

void
ResourceManager::SetGlobalVector2f(const char *name, const glm::vec2 &value) {
    for (auto &[key, shader] : Shaders) { shader->SetVector2f(name, value); }
}

void
ResourceManager::SetGlobalVector3f(const char *name, const glm::vec3 &value) {
    for (auto &[key, shader] : Shaders) { shader->SetVector3f(name, value); }
}

void
ResourceManager::SetGlobalVector4f(const char *name, const glm::vec4 &value) {
    for (auto &[key, shader] : Shaders) { shader->SetVector4f(name, value); }
}

void
ResourceManager::SetGlobalMatrix4(const char *name, const glm::mat4 &value) {
    for (auto &[key, shader] : Shaders) { shader->SetMatrix4(name, value); }
}
