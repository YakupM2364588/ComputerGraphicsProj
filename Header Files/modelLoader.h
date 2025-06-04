#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "model.h"
#include "texture.h"
#include "vertex.h"
#include "tiny_obj_loader.h"


//https://github.com/tinyobjloader/tinyobjloader
class ModelLoader {
public:
    static Model LoadOBJ(const std::string& filepath, const std::string& mtlBasePath = "") {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = mtlBasePath;
        reader_config.triangulate = true;

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                    filepath.c_str(), mtlBasePath.c_str(), true);
        if (!ret) throw std::runtime_error("Failed to load OBJ file.");

        std::vector<Mesh> meshes;

        for (size_t s = 0; s < shapes.size(); s++) {
            bool hasMaterials = !shapes[s].mesh.material_ids.empty();
            std::unordered_map<int, std::vector<size_t>> materialToFaceIndices;

            if (hasMaterials) {
                for (size_t f = 0; f < shapes[s].mesh.material_ids.size(); f++)
                    materialToFaceIndices[shapes[s].mesh.material_ids[f]].push_back(f);
            } else {
                for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
                    materialToFaceIndices[-1].push_back(f);
            }

            for (const auto& materialFaces : materialToFaceIndices) {
                const auto& faceIndices = materialFaces.second;

                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;

                for (size_t faceIdx : faceIndices) {
                    size_t fv = shapes[s].mesh.num_face_vertices[faceIdx];
                    for (size_t v = 0; v < fv; v++) {
                        size_t idx_offset = shapes[s].mesh.indices.size() >= (faceIdx * fv + v + 1)
                                            ? faceIdx * fv + v : 0;
                        tinyobj::index_t idx = shapes[s].mesh.indices[idx_offset];

                        Vertex vertex;

                        if (idx.vertex_index >= 0) {
                            vertex.position = {
                                attrib.vertices[3 * idx.vertex_index + 0],
                                attrib.vertices[3 * idx.vertex_index + 1],
                                attrib.vertices[3 * idx.vertex_index + 2]
                            };
                        } else {
                            vertex.position = {0.0f, 0.0f, 0.0f};
                        }

                        if (idx.texcoord_index >= 0) {
                            vertex.texCoord = {
                                attrib.texcoords[2 * idx.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                            };
                        } else {
                            vertex.texCoord = {0.0f, 0.0f};
                        }

                        if (idx.normal_index >= 0 &&
                            idx.normal_index * 3 + 2 < attrib.normals.size()) {
                            vertex.normal = {
                                attrib.normals[3 * idx.normal_index + 0],
                                attrib.normals[3 * idx.normal_index + 1],
                                attrib.normals[3 * idx.normal_index + 2]
                            };
                        }

                        vertex.color = {1.0f, 1.0f, 1.0f};

                        vertices.push_back(vertex);
                        indices.push_back(static_cast<unsigned int>(vertices.size() - 1));
                    }
                }


                if (!vertices.empty() && !indices.empty())
                    meshes.push_back(Mesh(vertices, indices, {}));
            }
        }

        return Model(meshes);
    }

private:

};

#endif
