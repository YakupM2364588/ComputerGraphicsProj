// Texture.cpp
#include "texture.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION //1 keer definen
#include "stb_image.h"

void Texture::Init(const std::string& filePath, const std::string& typeName)
{
    m_type = typeName;
    m_path = filePath;

    glGenTextures(1, &m_id);

    int width, height, channel;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channel, 0);

    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(data);
}

void Texture::BindToShader(const Shader& shader, const std::string& uniformName, unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
    shader.setInt(uniformName, unit);
}
