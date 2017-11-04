#pragma once

#include "GlObject.hh"
#include "Buffer.hh"
#include "VertexArray.hh"
#include "Texture.hh"

namespace cfl {
namespace gl {
inline void CompileShader(GLuint program, GLenum type, const char *source) {
    OGL(GLuint shader = glCreateShader(type));
    OGL(glShaderSource(shader, 1, &source, nullptr));
    OGL(glCompileShader(shader));

    GLint status, length;
    OGL(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));

    if (status == GL_FALSE) {
        OGL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

        std::vector<GLchar> buffer((ulong) length);
        OGL(glGetShaderInfoLog(shader, (GLsizei) buffer.size(), nullptr, buffer.data()));
        OGL(glDeleteShader(shader));
        std::cerr << "cfl::gl::CompileShader error: " << buffer.data() << std::endl;
        throw std::runtime_error("GLSL failed to compile");
    }

    OGL(glAttachShader(program, shader));
    OGL(glDeleteShader(shader));
}

class Shader {
    GLuint program;
    bool enabled = false;
    std::vector<std::string> defines;

protected:
    Shader(Shader const &r) = delete;

    Shader &operator=(Shader const &r) = delete;

public:
    Shader() : program() {}

    Shader(std::string const &vert, std::string const &frag, std::string const &geom = "") {
        OGL(program = glCreateProgram());

        OGL(glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_FALSE));

        CompileShader(program, GL_VERTEX_SHADER, vert.c_str());
        CompileShader(program, GL_FRAGMENT_SHADER, frag.c_str());

        if (geom.length() != 0) CompileShader(program, GL_GEOMETRY_SHADER, geom.c_str());

        OGL(glLinkProgram(program));

        // check for errors
        GLint status, length;
        OGL(glGetProgramiv(program, GL_LINK_STATUS, &status));
        if (status == GL_FALSE) {
            OGL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
            std::vector<GLchar> buffer(static_cast<ulong>(length));
            OGL(glGetProgramInfoLog(program, (GLsizei) buffer.size(), nullptr, buffer.data()));
            std::cerr << "GLSL linkage error: " << buffer.data() << std::endl;
            throw std::runtime_error("GLSL linkage failure");
        }
    }

    ~Shader() { if (program) glDeleteProgram(program); }

    void Bind() {
        if (program > 0) enabled = true;
        OGL(glUseProgram(program));
    }

    void Unbind() {
        enabled = false;
        OGL(glUseProgram(0));
    }

    GLuint ProgramHandle() const { return program; }

    GLint GetUniformLocation(std::string const &name) const {
        return OGL(glGetUniformLocation(program, name.c_str()));
    }

    void Uniform(std::string const &name, int scalar) const {
        OGL(glProgramUniform1i(program, GetUniformLocation(name), scalar));
    }

    void Uniform(std::string const &name, float scalar) const {
        OGL(glProgramUniform1f(program, GetUniformLocation(name), scalar));
    }

    void Uniform(std::string const &name, vec2 const &vec) const {
        OGL(glProgramUniform2fv(program, GetUniformLocation(name), 1, glm::value_ptr(vec)));
    }

    void Uniform(std::string const &name, vec3 const &vec) const {
        OGL(glProgramUniform3fv(program, GetUniformLocation(name), 1, glm::value_ptr(vec)));
    }

    void Uniform(std::string const &name, vec4 const &vec) const {
        OGL(glProgramUniform4fv(program, GetUniformLocation(name), 1, glm::value_ptr(vec)));
    }

    void Uniform(std::string const &name, mat3 const &mat) const {
        OGL(glProgramUniformMatrix3fv(program, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)));
    }

    void Uniform(std::string const &name, mat4 const &mat) const {
        OGL(glProgramUniformMatrix4fv(program, GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)));
    }

    void Uniform(std::string const &name, std::vector<int> const &values, GLsizei N = -1) const {
        OGL(glProgramUniform1iv(program, GetUniformLocation(name), N == -1 ? static_cast<GLsizei>(values.size()) : N,
                                values.data()));
    }

    void Uniform(std::string const &name, std::vector<float> const &values, GLsizei N = -1) const {
        OGL(glProgramUniform1fv(program, GetUniformLocation(name), N == -1 ? static_cast<GLsizei>(values.size()) : N,
                                values.data()));
    }

    void Uniform(std::string const &name, std::vector<vec2> const &values, GLsizei N = -1) const {
        OGL(glProgramUniform2fv(program, GetUniformLocation(name), N == -1 ? static_cast<GLsizei>(values.size()) : N,
                                glm::value_ptr(values[0])));
    }

    void Uniform(std::string const &name, std::vector<vec3> const &values, GLsizei N = -1) const {
        OGL(glProgramUniform3fv(program, GetUniformLocation(name), N == -1 ? static_cast<GLsizei>(values.size()) : N,
                                glm::value_ptr(values[0])));
    }

    void Uniform(std::string const &name, std::vector<mat3> const &values, GLsizei N = -1) const {
        OGL(glProgramUniformMatrix3fv(program, GetUniformLocation(name),
                                      N == -1 ? static_cast<GLsizei>(values.size()) : N,
                                      GL_FALSE, glm::value_ptr(values[0])));
    }

    void Uniform(std::string const &name, std::vector<mat4> const &values, GLsizei N = -1) const {
        OGL(glProgramUniformMatrix4fv(program, GetUniformLocation(name),
                                      N == -1 ? static_cast<GLsizei>(values.size()) : N,
                                      GL_FALSE, glm::value_ptr(values[0])));
    }

    void Texture(std::string const &name, GLenum target, GLenum unit, GlTextureBase const &tex) const {
        OGL(glActiveTexture(GL_TEXTURE0 + unit));
        OGL(glBindTexture(tex.target, tex));
        OGL(glProgramUniform1i(program, GetUniformLocation(name), unit));
    }
};
}
}
