#include "ShaderUtils.hpp"
#include <iostream>

static const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 FragPos;
out vec3 Normal;

void main() {
    FragPos = vec3(u_Model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_Model))) * aNormal;
    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 color = vec3(0.2, 0.6, 0.9) * diff;
    FragColor = vec4(color, 1.0);
}
)";

GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* srcPtr = source.c_str();
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader Compilation Failed:\n" << log << std::endl;
    }

    return shader;
}

GLuint createShaderProgram() {
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Shader Linking Failed:\n" << log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}
