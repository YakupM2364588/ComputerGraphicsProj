#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class BezierCurve {
private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> curvePoints;
    std::vector<GLfloat> curveVertices;
    void CalculateCurvePoints(int steps);
    void CreateCurveVertices();
public:
    BezierCurve(const std::vector<glm::vec3>& points);
    const std::vector<glm::vec3>& getCurvePoints() const;
    std::vector<GLfloat> getCurveVertices();
};