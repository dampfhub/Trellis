#include "texture.h"

#include <iostream>

using std::shared_ptr;

Texture2D::Texture2D(
    unsigned int   height,
    unsigned int   width,
    unsigned char *data,
    uint64_t       ImageUid,
    unsigned int   internal_format,
    unsigned int   image_format)
    : Width(width)
    , Height(height)
    , ImageUID(ImageUid)
    , Internal_Format(internal_format)
    , Image_Format(image_format)
    , Wrap_S(GL_REPEAT)
    , Wrap_T(GL_REPEAT)
    , Filter_Min(GL_NEAREST)
    , Filter_Max(GL_NEAREST) {
    // create Texture
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        Internal_Format,
        width,
        height,
        0,
        Image_Format,
        GL_UNSIGNED_BYTE,
        data);
    // set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_Max);
    // unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
Texture2D::Bind() const {
    glBindTexture(GL_TEXTURE_2D, ID);
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &ID);
}

std::shared_ptr<Texture2D>
Texture2D::Create(
    unsigned int   width,
    unsigned int   height,
    unsigned char *data,
    uint64_t       ImageUid,
    unsigned int   internal_format,
    unsigned int   image_format) {
    return shared_ptr<Texture2D>(
        new Texture2D(height, width, data, ImageUid, internal_format, image_format));
}
