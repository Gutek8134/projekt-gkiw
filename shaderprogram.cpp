#include "shaderprogram.h"
#include <stdio.h>

char *ShaderProgram::readFile(const char *filename)
{
    int filesize;
    FILE *file;
    char *result;

    file = fopen(filename, "rb");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
        filesize = ftell(file);
        fseek(file, 0, SEEK_SET);
        result = new char[filesize + 1];
        int readsize = fread(result, 1, filesize, file);
        result[filesize] = 0;
        fclose(file);

        return result;
    }

    return NULL;
}

// The method reads a shader code, compiles it and returns a corresponding handle
GLuint ShaderProgram::loadShader(GLenum shaderType, const char *fileName)
{
    // Create a shader handle
    GLuint shader = glCreateShader(shaderType); // shaderType to GL_VERTEX_SHADER, GL_GEOMETRY_SHADER lub GL_FRAGMENT_SHADER
    // Read a shader source file into a string
    const GLchar *shaderSource = readFile(fileName);
    // Associate source code with the shader handle
    glShaderSource(shader, 1, &shaderSource, NULL);
    // Compile source code
    glCompileShader(shader);
    // Delete source code from memory (it is no longer needed)
    delete[] shaderSource;

    // Download a compilation error log and display it
    int infologLength = 0;
    int charsWritten = 0;
    char *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 1)
    {
        infoLog = new char[infologLength];
        glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        delete[] infoLog;
    }

    // Return shader handle
    return shader;
}

ShaderProgram::ShaderProgram(const char *vertexShaderFile, const char *fragmentShaderFile, const char *geometryShaderFile)
{
    // Load vertex shader
    printf("Loading vertex shader...\n");
    vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderFile);

    // Load geometry shader
    if (geometryShaderFile != NULL)
    {
        printf("Loading geometry shader...\n");
        geometryShader = loadShader(GL_GEOMETRY_SHADER, geometryShaderFile);
    }
    else
    {
        geometryShader = 0;
    }

    // Load fragment shader
    printf("Loading fragment shader...\n");
    fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderFile);

    // Generate shader program handle
    shaderProgram = glCreateProgram();

    // Attach shaders and link shader program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if (geometryShaderFile != NULL)
        glAttachShader(shaderProgram, geometryShader);
    glLinkProgram(shaderProgram);

    // Download an error log and display it
    int infologLength = 0;
    int charsWritten = 0;
    char *infoLog;

    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 1)
    {
        infoLog = new char[infologLength];
        glGetProgramInfoLog(shaderProgram, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        delete[] infoLog;
    }

    printf("Shader program created \n");
}

ShaderProgram::~ShaderProgram()
{
    // Detach shaders from program
    glDetachShader(shaderProgram, vertexShader);
    if (geometryShader != 0)
        glDetachShader(shaderProgram, geometryShader);
    glDetachShader(shaderProgram, fragmentShader);

    // Delete shaders
    glDeleteShader(vertexShader);
    if (geometryShader != 0)
        glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    // Delete program
    glDeleteProgram(shaderProgram);
}

// Make the shader program active
void ShaderProgram::use()
{
    glUseProgram(shaderProgram);
}

// Get the slot number corresponding to the uniform variableName
GLuint ShaderProgram::getUniformLocation(const char *variableName)
{
    return glGetUniformLocation(shaderProgram, variableName);
}

// Get the slot number corresponding to the attribute variableName
GLuint ShaderProgram::getAttributeLocation(const char *variableName)
{
    return glGetAttribLocation(shaderProgram, variableName);
}
