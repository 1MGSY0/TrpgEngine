#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

GLuint createShaderProgram();  // Uses default hardcoded vertex/fragment strings
GLuint compileShader(GLenum type, const std::string& source);