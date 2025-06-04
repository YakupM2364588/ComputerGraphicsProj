#ifndef RAILWAY_H
#define RAILWAY_H
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "model.h"
#include "bezierCurvesPath.h"

class Railway {
public:
    Railway(const std::string& railModelPath, const std::string& mtlBasePath);
    void Draw(Shader& shader);
    const std::vector<glm::vec3>& GetRailPositions() const;
    const std::vector<float>& GetRailDistances() const;
    BezierCurvePath& GetPath();

private:
    Model m_railPiece;
    BezierCurvePath m_path;
    std::vector<glm::vec3> m_railPositions;
    std::vector<float> m_railDistances;

    void GenerateRailPositions();
    static std::vector<std::vector<glm::vec3>> CreateControlPoints();
};


#endif //RAILWAY_H
