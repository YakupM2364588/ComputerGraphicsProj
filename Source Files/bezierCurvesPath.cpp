#include "bezierCurvesPath.h"

BezierCurvePath::BezierCurvePath(const std::vector<std::vector<glm::vec3>>& curveControlPoints)
{
    // Create each Bezier curve
    for (const auto& points : curveControlPoints) {
        curves.emplace_back(points);
    }

    createAllCurveVertices();
}

const std::vector<BezierCurve>& BezierCurvePath::getAllCurves() const {
    return curves;
}


const std::vector<GLfloat>& BezierCurvePath::getAllCurvesVertices() const
{
    return allCurvesVertices;
}

std::vector<glm::vec3> BezierCurvePath::GetAllCurvePoints()
{
    std::vector<glm::vec3> allPoints;
    for (auto& curve : curves) {
        allPoints.insert(allPoints.end(), curve.getCurvePoints().begin(), curve.getCurvePoints().end());
    }
    return allPoints;
}

void BezierCurvePath::createAllCurveVertices()
{
    for (const auto& point : GetAllCurvePoints()) {
        allCurvesVertices.push_back(point.x);
        allCurvesVertices.push_back(point.y);
        allCurvesVertices.push_back(point.z);
    }
}
