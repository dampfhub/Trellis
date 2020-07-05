#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>
#include <glad/glad.h>
#include <memory>

#include "sqlite_handler.h"

// Texture2D is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class Texture2D {
public:
    static std::shared_ptr<Texture2D> Create(
        unsigned int   width,
        unsigned int   height,
        unsigned char *data,
        uint64_t       ImageUid,
        unsigned int   internal_format = GL_RGB,
        unsigned int   image_format    = GL_RGB);

    Texture2D(const Texture2D &) = delete;
    Texture2D &operator=(const Texture2D &) = delete;

    ~Texture2D();

    // texture image dimensions
    unsigned int Width, Height; // width and height of loaded image in pixels
    // Image uid for server lookup
    uint64_t ImageUID;
    // texture Format
    unsigned int Internal_Format; // format of texture object
    unsigned int Image_Format;    // format of loaded image
    // texture configuration
    unsigned int Wrap_S;     // wrapping mode on S axis
    unsigned int Wrap_T;     // wrapping mode on T axis
    unsigned int Filter_Min; // filtering mode if texture pixels < screen pixels
    unsigned int Filter_Max; // filtering mode if texture pixels > screen pixels

    // binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;

private:
    Texture2D(
        unsigned int   height,
        unsigned int   width,
        unsigned char *data,
        uint64_t       ImageUid,
        unsigned int   internal_format = GL_RGB,
        unsigned int   image_format    = GL_RGB);

    unsigned int ID{};
};

#endif
