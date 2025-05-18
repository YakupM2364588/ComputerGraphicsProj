#pragma once
#include "bezierCurve.h"
class BezierCurvePath {
private:
    std::vector<BezierCurve> curves;
    std::vector<GLfloat> allCurvesVertices;
    std::vector<float> curveLengths;
    float totalLength;

    void createAllCurveVertices();
    std::vector<glm::vec3> GetAllCurvePoints();
public:
    BezierCurvePath(const std::vector<std::vector<glm::vec3>>& curveControlPoints);
    const std::vector<BezierCurve>& BezierCurvePath::getAllCurves() const;
    const std::vector<GLfloat>& getAllCurvesVertices() const;
    glm::vec3 Evaluate(float s);
    float getTotalLength() const;
};