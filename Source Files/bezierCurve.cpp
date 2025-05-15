#include "bezierCurve.h"

BezierCurve::BezierCurve(const std::vector<glm::vec3>& points) : controlPoints(points)
{
	CalculateCurvePoints(100000);
	CreateCurveVertices();
}

void BezierCurve::CalculateCurvePoints(int steps)
{	
	const glm::vec3& P0 = controlPoints[0];
	const glm::vec3& P1 = controlPoints[1];
	const glm::vec3& P2 = controlPoints[2];
	const glm::vec3& P3 = controlPoints[3];
	
	glm::vec3 A = -P0 + 3.0f * P1 - 3.0f * P2 + P3;
	glm::vec3 B = 3.0f * P0 - 6.0f * P1 + 3.0f * P2;
	glm::vec3 C = -3.0f * P0 + 3.0f * P1;
	glm::vec3 D = P0;

	float dt = 1.0f / steps;
	float dt2 = dt * dt;
	float dt3 = dt2 * dt;
	
	glm::vec3 S = D;
	glm::vec3 dS = A * dt3 + B * dt2 + C * dt;
	glm::vec3 d2S = 6.0f * A * dt3 + 2.0f * B * dt2;
	glm::vec3 d3S = 6.0f * A * dt3;
	
	for (int i = 0; i <= steps; ++i) {
		curvePoints.push_back(S);
		S += dS;
		dS += d2S;
		d2S += d3S;
	}    
}

void BezierCurve::CreateCurveVertices()
{
	for (const auto& point : curvePoints) {
		curveVertices.push_back(point.x);
		curveVertices.push_back(point.y);
		curveVertices.push_back(point.z);
	}
}

const std::vector<glm::vec3>& BezierCurve::getCurvePoints() const 
{ 
	return curvePoints; 
}

std::vector<GLfloat> BezierCurve::getCurveVertices()
{
	return curveVertices;
}
