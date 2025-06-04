
#include "train.h"

Train::Train(const std::string& modelPath, const std::string& mtlPath)
    : model(modelPath, mtlPath) {
    model.SetScale(glm::vec3(0.07f));
}
float SafeAcos(float val) {
    if (val < -1.0f) return acos(-1.0f);
    if (val >  1.0f) return acos(1.0f);
    return acos(val);
}

//Update de positie van de trein, bij elke delta tijd
void Train::Update(float deltaTime, BezierCurvePath& path) {
    if (speed > 0.0f) {
        m_accumulatedTime += deltaTime;

        float totalLength = path.getTotalLength();
        float t = glm::mod(m_accumulatedTime * speed, totalLength);
        if (t < 0.0f) t = 0.0f;
        glm::vec3 trainPos = path.Evaluate(t);

        float nextT = std::min(t + 1, totalLength);
        glm::vec3 nextPos = path.Evaluate(nextT);

        glm::vec3 tangent = glm::normalize(nextPos - trainPos);
        glm::vec3 tangentXZ = glm::vec3(tangent.x, 0.0f, tangent.z);
        glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

        //theta = acos (V·U / |U||V|) de draaing rond de Y as
        float dotYaw = glm::dot( tangentXZ, zAxis);
        float lenYaw  = glm::length(tangent) * glm::length(zAxis);
        float yaw = SafeAcos(dotYaw / lenYaw);
        if (tangent.x < 0) yaw = -yaw;


        //Beetje naar schuiven zodat het op de rails zit
        glm::vec3 trackRight = glm::normalize(glm::cross(tangent, glm::vec3(0.0f, 1.0f, 0.0f)));
        float offset = -1.3f;

        glm::vec3 adjustedTrainPos = trainPos + (trackRight * offset);
        adjustedTrainPos.y += 0.15f;

        model.SetPosition(adjustedTrainPos);
        model.SetRotation(glm::vec3(0, yaw, 0));

        float frontOffset = 3.0f;
        frontPosition = adjustedTrainPos + tangent * frontOffset;
    }
}

void Train::Draw(Shader& shader) {
    model.Draw(shader);
}

glm::vec3 Train::GetFrontPosition() const
{
    return frontPosition;
}
