//
// Created by p0l on 5/19/25.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include<glad/glad.h>
#include "shaderClass.h"
class Texture {
public:

    Texture() = default;
    void Init(const std::string& filePath, const std::string& typeName);
    void BindToShader(const Shader& shader, const std::string& uniformName, unsigned int unit = 0) const ;
    GLuint m_id;
    std::string m_uniformName;
    std::string m_type;
    std::string m_path;
};

#endif //TEXTURE_H
