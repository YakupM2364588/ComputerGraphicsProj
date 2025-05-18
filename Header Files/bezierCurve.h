#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class BezierCurve {
private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> curvePoints;
    std::vector<GLfloat> curveVertices;
    float totalLength;
    std::vector<float> arcLengthTable;
    std::vector<float> tValues;

    void CalculateCurvePoints(int steps);
    void CreateCurveVertices();
	void CreateArcLengthTable();
public:
    BezierCurve(const std::vector<glm::vec3>& points);
    const std::vector<glm::vec3>& getCurvePoints() const;
    std::vector<GLfloat> getCurveVertices();
    glm::vec3 Evaluate(float t) const;
	float getTFromArcLength(float length) const;
    float getTotalLength() const;
};