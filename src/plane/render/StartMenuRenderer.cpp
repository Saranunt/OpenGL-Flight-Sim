#include "StartMenuRenderer.h"

#include <stb_image.h>
#include <iostream>

namespace plane::render
{
    void StartMenuRenderer::Initialize(const std::string& imagePath)
    {
        // Create fullscreen quad
        float vertices[] = {
            // Positions   // TexCoords
            -1.0f,  1.0f,  0.0f, 1.0f,  // Top-left
            -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
             1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
             1.0f,  1.0f,  1.0f, 1.0f   // Top-right
        };

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        CreateShader();
        LoadTexture(imagePath);
    }

    void StartMenuRenderer::CreateShader()
    {
        const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            
            out vec2 TexCoord;
            
            void main()
            {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        const char* fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            in vec2 TexCoord;
            
            uniform sampler2D menuTexture;
            
            void main()
            {
                FragColor = texture(menuTexture, TexCoord);
            }
        )";

        // Compile vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        // Check for vertex shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Compile fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // Check for fragment shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Link shaders
        shaderProgram_ = glCreateProgram();
        glAttachShader(shaderProgram_, vertexShader);
        glAttachShader(shaderProgram_, fragmentShader);
        glLinkProgram(shaderProgram_);

        // Check for linking errors
        glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shaderProgram_, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void StartMenuRenderer::LoadTexture(const std::string& imagePath)
    {
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);

        // Set texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Flip vertically during load (standard for OpenGL)
        stbi_set_flip_vertically_on_load(true);
        
        // Load image
        int width, height, nrChannels;
        unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);
        
        if (data)
        {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            // delete[] data;
        }
        else
        {
            std::cerr << "Failed to load start menu texture: " << imagePath << std::endl;
        }
        
        stbi_image_free(data);
    }

    void StartMenuRenderer::Shutdown()
    {
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
            glDeleteBuffers(1, &vbo_);
        }
        if (texture_ != 0)
        {
            glDeleteTextures(1, &texture_);
        }
        if (shaderProgram_ != 0)
        {
            glDeleteProgram(shaderProgram_);
        }
    }

    void StartMenuRenderer::Render(int screenWidth, int screenHeight) const
    {
        if (vao_ == 0 || shaderProgram_ == 0 || texture_ == 0)
        {
            return;
        }

        // Use fullscreen viewport
        glViewport(0, 0, screenWidth, screenHeight);
        
        glDisable(GL_DEPTH_TEST);
        
        glUseProgram(shaderProgram_);
        
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glUniform1i(glGetUniformLocation(shaderProgram_, "menuTexture"), 0);
        
        // Render quad
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
        
        glEnable(GL_DEPTH_TEST);
    }
}
