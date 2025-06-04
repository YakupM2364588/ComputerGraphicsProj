#ifndef VERTEX_H
#define VERTEX_H

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 color = glm::vec3(1.0f);
    bool operator==(const Vertex& other) const {
        return position.x == other.position.x &&
               position.y == other.position.y &&
               position.z == other.position.z &&
               texCoord.x == other.texCoord.x &&
               texCoord.y == other.texCoord.y &&
               normal.x == other.normal.x &&
               normal.y == other.normal.y &&
               normal.z == other.normal.z;
    }
};

#endif // VERTEX_H