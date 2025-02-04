/*
Based on the following code:

Shader class
- loading Shader source code, Shader Program creation

N.B. ) adaptation of https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/shader.h

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2023/2024
Master degree in Computer Science
Universita' degli Studi di Milano
*/

#pragma once

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utils/nocopy.h>

using std::string;
using std::ifstream;
using std::stringstream;
using std::cout;
using std::endl;

class Shader : NoCopy
{
public:
    explicit Shader(const string& vertexPath, const string& fragmentPath): NoCopy{}
    {
        const GLuint fragmentShader = compileFragmentShader(fragmentPath);
        const GLuint vertexShader = compileVertexShader(vertexPath);
        // Step 3: Shader Program creation
        this->_program = glCreateProgram();
        std::cout << "Program created: " << this->_program << std::endl;
        glAttachShader(this->_program, vertexShader);
        glAttachShader(this->_program, fragmentShader);
        glLinkProgram(this->_program);
        // check linking errors
        checkCompileErrors(this->_program, GL_PROGRAM);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    ~Shader()
    {
        freeGPUResources();
    }

    Shader(Shader&& other) noexcept: NoCopy{}, _program{other._program}
    {
        other._program = 0;
    };

    Shader& operator=(Shader&& other) noexcept
    {
        freeGPUResources();
        this->_program = other._program;

        other._program = 0;
        return *this;
    };

    void use() const
    {
        glUseProgram(this->_program);
    }

    GLuint program() const { return this->_program; }

    void validateProgram() const
    {
        glValidateProgram(this->_program);
        GLint isValid;
        glGetProgramiv(this->_program, GL_VALIDATE_STATUS, &isValid);
        if (isValid == GL_FALSE)
        {
            GLchar infoLog[512];
            glGetProgramInfoLog(this->_program, 512, nullptr, infoLog);
            std::cerr << "Program validation failed for program " << this->program() << ": " << infoLog << std::endl;
            throw std::runtime_error("Program validation failed");
        }
    }

private:
    GLuint _program;

    void freeGPUResources()
    {
        if (_program)
        {
            glDeleteProgram(this->_program);
            _program = 0;
        }
    }

    static GLuint compileVertexShader(const string& shaderPath)
    {
        return compileSingleShader(shaderPath, GL_VERTEX_SHADER);
    };

    static GLuint compileFragmentShader(const string& shaderPath)
    {
        return compileSingleShader(shaderPath, GL_FRAGMENT_SHADER);
    };

    static GLuint compileSingleShader(const string& shaderPath, GLuint shaderType)
    {
        string shaderString;
        ifstream shaderFile;

        shaderFile.exceptions(ifstream::failbit | ifstream::badbit);
        try
        {
            shaderFile.open(shaderPath);
            stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            shaderString = shaderStream.str();
        }
        catch (ifstream::failure& e)
        {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
            cout << "Path: " << shaderPath << endl;
        }

        const GLchar* shaderCode = shaderString.c_str();

        GLuint shader;

        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        checkCompileErrors(shader, shaderType);

        return shader;
    }

    static void checkCompileErrors(const GLuint shader, const GLuint type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER)
        {
            const string& typeStr = type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader";

            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
                cout << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << typeStr << "|\n" << infoLog <<
                    "\n| -- --------------------------------------------------- -- |" << endl;
            }
        }
        else if (type == GL_PROGRAM)
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
                cout << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << "PROGRAM" << "|\n" << infoLog <<
                    "\n| -- --------------------------------------------------- -- |" << endl;
            }
        }
    }
};
