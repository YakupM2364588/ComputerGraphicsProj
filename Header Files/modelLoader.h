#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "model.h"
#include "texture.h"
#include "vertex.h"
#include "tiny_obj_loader.h"


class ModelLoader {
public:
    static Model LoadOBJ(const std::string& filepath, const std::string& mtlBasePath = "") {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        
        std::unordered_map<std::string, Texture> loadedTextures;
        
        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = mtlBasePath;
        reader_config.triangulate = true;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                   filepath.c_str(), mtlBasePath.c_str(), true);

        if (!warn.empty()) {
            std::cout << "TinyObjLoader warning: " << warn << std::endl;
        }

        if (!err.empty()) {
            std::cerr << "TinyObjLoader error: " << err << std::endl;
        }

        if (!ret) {
            throw std::runtime_error("Failed to load OBJ file: " + filepath);
        }

        std::vector<Mesh> meshes;

        for (size_t s = 0; s < shapes.size(); s++) {
            bool hasMaterials = !shapes[s].mesh.material_ids.empty();

            std::unordered_map<int, std::vector<size_t>> materialToFaceIndices;

            if (hasMaterials) {
                for (size_t f = 0; f < shapes[s].mesh.material_ids.size(); f++) {
                    int materialID = shapes[s].mesh.material_ids[f];
                    materialToFaceIndices[materialID].push_back(f);
                }
            } else {
                // No materials, all faces use the default material (-1)
                for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                    materialToFaceIndices[-1].push_back(f);
                }
            }

            // Process each material group
            for (const auto& materialFaces : materialToFaceIndices) {
                int materialID = materialFaces.first;
                const auto& faceIndices = materialFaces.second;

                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;
                std::vector<Texture> textures;

                // Maps from vertex position/texcoord/normal to index in our vertex array
                std::unordered_map<Vertex, uint32_t, VertexHash> uniqueVertices;

                // Process all faces with this material
                for (size_t faceIdx : faceIndices) {
                    // Number of vertices per face
                    size_t fv = shapes[s].mesh.num_face_vertices[faceIdx];

                    // Get all vertices in this face
                    for (size_t v = 0; v < fv; v++) {
                        size_t idx_offset = shapes[s].mesh.indices.size() >= (faceIdx * fv + v + 1)
                                          ? faceIdx * fv + v
                                          : 0;
                        tinyobj::index_t idx = shapes[s].mesh.indices[idx_offset];

                        // Create vertex with position, texture coordinates, and normal
                        Vertex vertex;

                        // Position
                        if (idx.vertex_index >= 0 &&
                            idx.vertex_index * 3 + 2 < attrib.vertices.size()) {
                            vertex.position = {
                                attrib.vertices[3 * idx.vertex_index + 0],
                                attrib.vertices[3 * idx.vertex_index + 1],
                                attrib.vertices[3 * idx.vertex_index + 2]
                            };
                        } else {
                            vertex.position = {0.0f, 0.0f, 0.0f};
                            std::cerr << "Warning: Invalid vertex index!" << std::endl;
                        }

                        // Texture coordinates (if available)
                        if (idx.texcoord_index >= 0 &&
                            idx.texcoord_index * 2 + 1 < attrib.texcoords.size()) {
                            vertex.texCoord = {
                                attrib.texcoords[2 * idx.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // Flip Y for OpenGL
                            };
                        } else {
                            vertex.texCoord = {0.0f, 0.0f};
                        }

                        // Normal (if available)
                        if (idx.normal_index >= 0 &&
                            idx.normal_index * 3 + 2 < attrib.normals.size()) {
                            vertex.normal = {
                                attrib.normals[3 * idx.normal_index + 0],
                                attrib.normals[3 * idx.normal_index + 1],
                                attrib.normals[3 * idx.normal_index + 2]
                            };
                        } else {
                            // We'll calculate face normals later if needed
                            vertex.normal = {0.0f, 1.0f, 0.0f};
                        }

                        // Add vertex color from material if available
                        if (materialID >= 0 && materialID < materials.size()) {
                            const auto& material = materials[materialID];
                            vertex.color = {
                                material.diffuse[0],
                                material.diffuse[1],
                                material.diffuse[2]
                            };
                        } else {
                            // Default color (white)
                            vertex.color = {1.0f, 1.0f, 1.0f};
                        }

                        // Check if we've seen this vertex before
                        if (uniqueVertices.count(vertex) == 0) {
                            // If not, add it to our vertices and record its index
                            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                            vertices.push_back(vertex);
                        }

                        // Add the index
                        indices.push_back(uniqueVertices[vertex]);
                    }
                }

                // Calculate normals if not provided
                if (attrib.normals.size() == 0) {
                    CalculateNormals(vertices, indices);
                }

                // Load material textures if available
                if (materialID >= 0 && materialID < materials.size()) {
                    const auto& material = materials[materialID];

                    // Diffuse texture
                    if (!material.diffuse_texname.empty()) {
                        std::string texturePath = mtlBasePath + material.diffuse_texname;
                        std::cout << "Loading diffuse texture: " << texturePath << std::endl;

                        // Check if texture is already loaded
                        if (loadedTextures.find(texturePath) == loadedTextures.end()) {
                            Texture diffuseTexture;
                            diffuseTexture.Init(texturePath, "texture_diffuse");
                            loadedTextures[texturePath] = diffuseTexture;
                        }

                        textures.push_back(loadedTextures[texturePath]);
                    } else {
                        std::cout << "No diffuse texture for material ID " << materialID << std::endl;
                    }

                    // Specular texture
                    if (!material.specular_texname.empty()) {
                        std::string texturePath = mtlBasePath + material.specular_texname;
                        std::cout << "Loading specular texture: " << texturePath << std::endl;

                        if (loadedTextures.find(texturePath) == loadedTextures.end()) {
                            Texture specularTexture;
                            specularTexture.Init(texturePath, "texture_specular");
                            loadedTextures[texturePath] = specularTexture;
                        }

                        textures.push_back(loadedTextures[texturePath]);
                    }

                    // Normal map
                    if (!material.normal_texname.empty()) {
                        std::string texturePath = mtlBasePath + material.normal_texname;
                        std::cout << "Loading normal texture: " << texturePath << std::endl;

                        if (loadedTextures.find(texturePath) == loadedTextures.end()) {
                            Texture normalTexture;
                            normalTexture.Init(texturePath, "texture_normal");
                            loadedTextures[texturePath] = normalTexture;
                        }

                        textures.push_back(loadedTextures[texturePath]);
                    }
                }

                // Create mesh and add to meshes vector
                if (!vertices.empty() && !indices.empty()) {
                    std::cout << "Creating mesh with " << vertices.size() << " vertices, "
                              << indices.size() << " indices, and " << textures.size() << " textures" << std::endl;
                    meshes.push_back(Mesh(vertices, indices, textures));
                }
            }
        }

        if (meshes.empty()) {
            std::cerr << "Warning: No meshes were created from the model!" << std::endl;
        }

        // Create model from all meshes
        return Model(meshes);
    }

private:
    // Calculate normals for faces where they're not provided
    static void CalculateNormals(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        // Reset all normals
        for (auto& vertex : vertices) {
            vertex.normal = glm::vec3(0.0f);
        }

        // Calculate normals for each face
        for (size_t i = 0; i < indices.size(); i += 3) {
            unsigned int idx1 = indices[i];
            unsigned int idx2 = indices[i + 1];
            unsigned int idx3 = indices[i + 2];

            glm::vec3 v1 = vertices[idx1].position;
            glm::vec3 v2 = vertices[idx2].position;
            glm::vec3 v3 = vertices[idx3].position;

            glm::vec3 edge1 = v2 - v1;
            glm::vec3 edge2 = v3 - v1;
            glm::vec3 normal = glm::cross(edge1, edge2);

            // Add this normal to all three vertices
            vertices[idx1].normal += normal;
            vertices[idx2].normal += normal;
            vertices[idx3].normal += normal;
        }

        // Normalize all normals
        for (auto& vertex : vertices) {
            vertex.normal = glm::normalize(vertex.normal);
        }
    }

    // Helper struct to hash Vertex objects for the unordered_map
    struct VertexHash {
        size_t operator()(const Vertex& vertex) const {
            // Simple hash function for Vertex
            size_t h1 = std::hash<float>()(vertex.position.x);
            size_t h2 = std::hash<float>()(vertex.position.y);
            size_t h3 = std::hash<float>()(vertex.position.z);
            size_t h4 = std::hash<float>()(vertex.texCoord.x);
            size_t h5 = std::hash<float>()(vertex.texCoord.y);
            size_t h6 = std::hash<float>()(vertex.normal.x);
            size_t h7 = std::hash<float>()(vertex.normal.y);
            size_t h8 = std::hash<float>()(vertex.normal.z);
            size_t h9 = std::hash<float>()(vertex.color.x);
            size_t h10 = std::hash<float>()(vertex.color.y);
            size_t h11 = std::hash<float>()(vertex.color.z);

            return ((((((((((h1 ^ (h2 << 1)) ^ (h3 << 1)) ^ (h4 << 1)) ^ (h5 << 1))
                   ^ (h6 << 1)) ^ (h7 << 1)) ^ (h8 << 1)) ^ (h9 << 1)) ^ (h10 << 1)) ^ (h11 << 1));
        }
    };
};

#endif