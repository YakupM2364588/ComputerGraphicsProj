//
// Created by p0l on 5/26/25.
//

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
void Train::Update(float deltaTime, BezierCurvePath& path) {
    if (speed > 0.0f) {
        accumulatedTime += deltaTime;

        float totalLength = path.getTotalLength();
        float dist = fmod(accumulatedTime * speed, totalLength);
        if (dist < 0.0f) dist = 0.0f;

        glm::vec3 trainPos = path.Evaluate(dist);

        float lookAheadDistance = 1.0f;
        float nextDist = std::min(dist + lookAheadDistance, totalLength);
        glm::vec3 nextPos = path.Evaluate(nextDist);
        glm::vec3 tangent = glm::normalize(nextPos - trainPos);

        // Using angle formula: theta = acos((v1 DOT v2))
        glm::vec3 tangentXZ = glm::vec3(tangent.x, 0.0f, tangent.z);
        glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);


        // Calculate yaw
        float dotYaw = glm::dot(tangent, zAxis);
        float lenYaw  = glm::length(tangent) * glm::length(zAxis);
        float yaw = SafeAcos(dotYaw / lenYaw);
        if (tangent.x < 0) yaw = -yaw;  // Preserve direction


        glm::vec3 trackRight = glm::normalize(glm::cross(tangent, glm::vec3(0.0f, 1.0f, 0.0f)));
        float lateralOffset = -1.3f;

        glm::vec3 adjustedTrainPos = trainPos + (trackRight * lateralOffset);
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
