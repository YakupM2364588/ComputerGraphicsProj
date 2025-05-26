
#include "railway.h"

Railway::Railway(const std::string& railModelPath, const std::string& mtlBasePath)
    : m_railPiece(railModelPath, mtlBasePath), m_path(CreateControlPoints()) {
    GenerateRailPositions();
}
void Railway::Draw(Shader& shader) {
    const float RailLength = 1.3f;
    float totalLength = m_path.getTotalLength();

    for (size_t i = 0; i < m_railDistances.size(); i++) {
        float s = m_railDistances[i];
        glm::vec3 pos = m_path.Evaluate(s);

        float nextS = std::min(s + RailLength * 0.1f, totalLength);
        glm::vec3 nextPos = m_path.Evaluate(nextS);
        glm::vec3 tangent = glm::normalize(nextPos - pos);

        float yaw = atan2(tangent.x, tangent.z);
        float horizontalLength = sqrt(tangent.x * tangent.x + tangent.z * tangent.z);
        float pitch = atan2(-tangent.y, horizontalLength);

        m_railPiece.SetPosition(pos + glm::vec3(0, 0.01f, 0));
        m_railPiece.SetRotation(glm::vec3(pitch, yaw, 0));
        m_railPiece.Draw(shader);
    }
}

void Railway::GenerateRailPositions() {
    const float RailLength = 1.3f;
    float totalLength = m_path.getTotalLength();

    m_railPositions.clear();
    m_railDistances.clear();

    for (float s = 0.0f; s < totalLength; s += RailLength) {
        m_railDistances.push_back(s);
        m_railPositions.push_back(m_path.Evaluate(s));
    }
}

std::vector<std::vector<glm::vec3>> Railway::CreateControlPoints() {
    std::vector<std::vector<glm::vec3>> segments;

    float radius = 30.0f;
    int numSegments = 8;

    for (int i = 0; i < numSegments; i++) {
        std::vector<glm::vec3> segment(4);

        float angle1 = (float)i / numSegments * 2.0f * M_PI;
        float angle2 = (float)(i + 1) / numSegments * 2.0f * M_PI;

        segment[0] = glm::vec3(cos(angle1) * radius, 0, sin(angle1) * radius);

        glm::vec3 tangent1 = glm::vec3(-sin(angle1), 0, cos(angle1)) * radius * 0.3f;
        segment[1] = segment[0] + tangent1;

        segment[3] = glm::vec3(cos(angle2) * radius, 0, sin(angle2) * radius);

        glm::vec3 tangent2 = glm::vec3(-sin(angle2), 0, cos(angle2)) * radius * 0.3f;
        segment[2] = segment[3] - tangent2;

        segments.push_back(segment);
    }

    return segments;
}

const std::vector<glm::vec3>& Railway::GetRailPositions() const {
    return m_railPositions;
}

const std::vector<float>& Railway::GetRailDistances() const {
    return m_railDistances;
}

BezierCurvePath& Railway::GetPath() {
    return m_path;
}