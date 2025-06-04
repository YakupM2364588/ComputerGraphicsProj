
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

//Draw de rails op de bezier pad met de juiste positie en rotatie
void Railway::Draw(Shader& shader) {
    float totalLength = m_path.getTotalLength();

    for (size_t i = 0; i < m_railDistances.size(); i++) {
        float t = m_railDistances[i];
        glm::vec3 pos = m_path.Evaluate(t);

        float nextT = std::min(t + 1.0f , totalLength);
        glm::vec3 nextPos = m_path.Evaluate(nextT);

        glm::vec3 tangent = glm::normalize(nextPos - pos);
        glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 tangentXZ = glm::vec3(tangent.x, 0.0f, tangent.z);

        //theta = acos (V·U / |U||V|) de draaing rond de Y as
        float dotYaw = glm::dot( tangentXZ, zAxis);
        float lenYaw  = glm::length(tangent) * glm::length(zAxis);
        float yaw = SafeAcos1(dotYaw / lenYaw);
        if (tangent.x < 0) yaw = -yaw;

        m_railPiece.SetPosition(pos);
        m_railPiece.SetRotation(glm::vec3(0, yaw, 0));
        m_railPiece.Draw(shader);
    }
}

//Genereren van railposities op vaste afstand(rail lengthe)
void Railway::GenerateRailPositions() {
    float totalLength = m_path.getTotalLength();

    m_railPositions.clear();
    m_railDistances.clear();

    for (float s = 0.0f; s < totalLength; s += 1.0f) {
        m_railDistances.push_back(s);
        m_railPositions.push_back(m_path.Evaluate(s));
    }
}

//Controle punten van de railway(fixed)
std::vector<std::vector<glm::vec3>> Railway::CreateControlPoints() {
    std::vector<std::vector<glm::vec3>> segments;
    segments.push_back({
        glm::vec3(0, 0, 0),
        glm::vec3(10, 0, 0),
        glm::vec3(20, 0, 0),
        glm::vec3(30, 0, 0)
    });

    segments.push_back({
        glm::vec3(30, 0, 0),
        glm::vec3(40, 0, 0),
        glm::vec3(50, 0, 10),
        glm::vec3(50, 0, 20)
    });

    segments.push_back({
        glm::vec3(50, 0, 20),
        glm::vec3(50, 0, 30),
        glm::vec3(50, 0, 40),
        glm::vec3(50, 0, 50)
    });

    segments.push_back({
        glm::vec3(50, 0, 50),
        glm::vec3(50, 0, 60),
        glm::vec3(40, 0, 70),
        glm::vec3(30, 0, 70)
    });

    segments.push_back({
        glm::vec3(30, 0, 70),
        glm::vec3(20, 0, 70),
        glm::vec3(10, 0, 70),
        glm::vec3(0, 0, 70)
    });

    segments.push_back({
        glm::vec3(0, 0, 70),
        glm::vec3(-10, 0, 70),
        glm::vec3(-20, 0, 60),
        glm::vec3(-20, 0, 50)
    });

    segments.push_back({
        glm::vec3(-20, 0, 50),
        glm::vec3(-20, 0, 40),
        glm::vec3(-20, 0, 30),
        glm::vec3(-20, 0, 20)
    });
    segments.push_back({
        glm::vec3(-20, 0, 20),
        glm::vec3(-20, 0, 10),
        glm::vec3(-10, 0, 0),
        glm::vec3(0, 0, 0)
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
