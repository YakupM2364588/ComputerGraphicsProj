#include "bezierCurve.h"

BezierCurve::BezierCurve(const std::vector<glm::vec3>& points) : controlPoints(points)
{
	CalculateCurvePoints(100000);
	CreateCurveVertices();
	CreateArcLengthTable();
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

void BezierCurve::CreateArcLengthTable()
{
	totalLength = 0.0f;

	arcLengthTable.push_back(0.0f);
	tValues.push_back(0.0f);

	for (size_t i = 1; i < curvePoints.size(); i++) {
		float segmentLength = glm::distance(curvePoints[i], curvePoints[i - 1]);
		totalLength += segmentLength;
		arcLengthTable.push_back(totalLength);
		tValues.push_back(static_cast<float>(i) / (curvePoints.size() - 1));
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

glm::vec3 BezierCurve::Evaluate(float t) const
{
	float u = 1.0f - t;
	float u2 = u * u;
	float u3 = u2 * u;
	float t2 = t * t;
	float t3 = t2 * t;

	return u3 * controlPoints[0] +
		3 * u2 * t * controlPoints[1] +
		3 * u * t2 * controlPoints[2] +
		t3 * controlPoints[3];
}

float BezierCurve::getTFromArcLength(float length) const
{
	if (length <= 0.0f) return 0.0f;
	if (length >= totalLength) return 1.0f;
		
	// Binary search for the segment
	int low = 0;
	int high = arcLengthTable.size() - 1;
	while (low < high) {
		int mid = (low + high) / 2;
		if (arcLengthTable[mid] < length) {
			low = mid + 1;
		}
		else {
			high = mid;
		}
	}
	if (low == 0) return tValues[0];
	if (low == arcLengthTable.size()) return tValues.back();
	float segmentLength = arcLengthTable[low] - arcLengthTable[low - 1];
	float segmentT = (length - arcLengthTable[low - 1]) / segmentLength;
	return tValues[low - 1] + segmentT * (tValues[low] - tValues[low - 1]);
}

float BezierCurve::getTotalLength() const
{
	return totalLength;
}

