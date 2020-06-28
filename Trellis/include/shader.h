#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

// General purpsoe sprite_shader object. Compiles from file, generates
// compile/link-time error messages and hosts several utility
// functions for easy management.
class Shader {
public:
    // constructor
    Shader(
        const char *vertexSource,
        const char *fragmentSource,
        const char *geometrySource = nullptr); // note: geometry source code is optional
    ~Shader();
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    // sets the current sprite_shader as active
    Shader &Use();

    // utility functions
    void SetFloat(const char *name, float value);

    void SetInteger(const char *name, int value);

    void SetVector2f(const char *name, float x, float y);

    void SetVector2f(const char *name, const glm::vec2 &value);

    void SetVector2i(const char *name, const glm::vec2 &value);

    void SetVector3f(const char *name, float x, float y, float z);

    void SetVector3f(const char *name, const glm::vec3 &value);

    void SetVector4f(const char *name, float x, float y, float z, float w);

    void SetVector4f(const char *name, const glm::vec4 &value);

    void SetMatrix4(const char *name, const glm::mat4 &matrix);

private:
    // state
    unsigned int ID;

    // checks if compilation or linking failed and if so, print the error logs
    void checkCompileErrors(unsigned int object, std::string type);
};

#endif