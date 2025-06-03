
#include "railway.h"

Railway::Railway(const std::string& railModelPath, const std::string& mtlBasePath)
    : m_railPiece(railModelPath, mtlBasePath), m_path(CreateControlPoints()) {
    GenerateRailPositions();
}
float SafeAcos1(float val) {
    if (val < -1.0f) return acos(-1.0f);
    if (val >  1.0f) return acos(1.0f);
    return acos(val);
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

        // Using angle formula: theta = acos((v1 DOT v2)/(|v1|*|v2|))
        glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

        // Calculate yaw
        float dotYaw = glm::dot(tangent, zAxis);
        float lenYaw  = glm::length(tangent) * glm::length(zAxis);
        float yaw = SafeAcos1(dotYaw / lenYaw);
        if (tangent.x < 0) yaw = -yaw;  // Preserve direction

        // Calculate pitch

        m_railPiece.SetPosition(pos);
        m_railPiece.SetRotation(glm::vec3(0, yaw, 0));
        m_railPiece.Draw(shader);
    }
}
void Railway::GenerateRailPositions() {
    const float RailLength = 0.5f;
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

    // Simple railway track with straight sections and gentle curves
    // All values are clean, round numbers that make sense

    // Segment 0: Straight section going forward
    segments.push_back({
        glm::vec3(0, 0, 0),      // Start
        glm::vec3(10, 0, 0),     // Control point 1
        glm::vec3(20, 0, 0),     // Control point 2
        glm::vec3(30, 0, 0)      // End
    });

    // Segment 1: Gentle right turn
    segments.push_back({
        glm::vec3(30, 0, 0),     // Start (continues from previous)
        glm::vec3(40, 0, 0),     // Straight out first
        glm::vec3(50, 0, 10),    // Start curving right
        glm::vec3(50, 0, 20)     // End of turn
    });

    // Segment 2: Another straight section
    segments.push_back({
        glm::vec3(50, 0, 20),    // Start
        glm::vec3(50, 0, 30),    // Control point 1
        glm::vec3(50, 0, 40),    // Control point 2
        glm::vec3(50, 0, 50)     // End
    });

    // Segment 3: Left turn back toward center
    segments.push_back({
        glm::vec3(50, 0, 50),    // Start
        glm::vec3(50, 0, 60),    // Straight out first
        glm::vec3(40, 0, 70),    // Start turning left
        glm::vec3(30, 0, 70)     // End of turn
    });

    // Segment 4: Straight section back
    segments.push_back({
        glm::vec3(30, 0, 70),    // Start
        glm::vec3(20, 0, 70),    // Control point 1
        glm::vec3(10, 0, 70),    // Control point 2
        glm::vec3(0, 0, 70)      // End
    });

    // Segment 5: Left turn to start going down
    segments.push_back({
        glm::vec3(0, 0, 70),     // Start
        glm::vec3(-10, 0, 70),   // Straight out first
        glm::vec3(-20, 0, 60),   // Start turning left
        glm::vec3(-20, 0, 50)    // End of turn
    });

    // Segment 6: Straight section going down
    segments.push_back({
        glm::vec3(-20, 0, 50),   // Start
        glm::vec3(-20, 0, 40),   // Control point 1
        glm::vec3(-20, 0, 30),   // Control point 2
        glm::vec3(-20, 0, 20)    // End
    });

    // Segment 7: Right turn toward bottom
    segments.push_back({
        glm::vec3(-20, 0, 20),   // Start
        glm::vec3(-20, 0, 10),   // Straight out first
        glm::vec3(-10, 0, 0),    // Start turning right
        glm::vec3(0, 0, 0)       // End - back to start!
    });

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
