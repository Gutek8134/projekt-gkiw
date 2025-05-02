#pragma once
#include <GL/glew.h>

class ShaderProgram
{
private:
    GLuint shaderProgram;                                       // Shader program handle
    GLuint vertexShader;                                        // Vertex shader handle
    GLuint geometryShader;                                      // Geometry shader handle
    GLuint fragmentShader;                                      // Fragment shader handle
    char *readFile(const char *filename);                       // File reading method
    GLuint loadShader(GLenum shaderType, const char *fileName); // Reads shader source file, compiles it and returns the corresponding handle
public:
    ShaderProgram(const char *vertexShaderFile, const char *fragmentShaderFile, const char *geometryShaderFile = NULL);
    ~ShaderProgram();
    void use();                                            // Turns on the shader program
    GLuint getUniformLocation(const char *variableName);   // Returns the slot number corresponding to the uniform variableName
    GLuint getAttributeLocation(const char *variableName); // Returns the slot number corresponding to the attribute variableName
};