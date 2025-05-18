#include "bezierCurvesPath.h"

BezierCurvePath::BezierCurvePath(const std::vector<std::vector<glm::vec3>>& curveControlPoints)
{
    // Create each Bezier curve
    for (const auto& points : curveControlPoints) {
        curves.emplace_back(points);
    }

    createAllCurveVertices();

    totalLength = 0.0f;
    for (auto& curve : curves) {
        curveLengths.push_back(curve.getTotalLength());
        totalLength += curve.getTotalLength();
    }
}

const std::vector<BezierCurve>& BezierCurvePath::getAllCurves() const {
    return curves;
}

const std::vector<GLfloat>& BezierCurvePath::getAllCurvesVertices() const
{
    return allCurvesVertices;
}

glm::vec3 BezierCurvePath::Evaluate(float s)
{
    if (s <= 0) return curves.front().Evaluate(0);
    if (s >= totalLength) return curves.back().Evaluate(1);

    float accumulatedLength = 0.0f;
    size_t curveIndex = 0;
    for (; curveIndex < curves.size(); curveIndex++) {
        if (s <= accumulatedLength + curveLengths[curveIndex]) {
            break;
        }
        accumulatedLength += curveLengths[curveIndex];
    }

    float curveDistance = s - accumulatedLength;
    return curves[curveIndex].Evaluate(curves[curveIndex].getTFromArcLength(curveDistance));
}

float BezierCurvePath::getTotalLength() const
{
	return totalLength;
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
