//
// Created by p0l on 5/26/25.
//

#include "train.h"

Train::Train(const std::string& modelPath, const std::string& mtlPath)
    : model(modelPath, mtlPath) {
    model.SetScale(glm::vec3(0.07f));
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

        glm::vec3 trainDirection = glm::normalize(nextPos - trainPos);

        if (glm::length(trainDirection) < 0.001f) {
            trainDirection = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        float trainYaw = atan2(trainDirection.x, trainDirection.z);
        float horizontalLength = sqrt(trainDirection.x * trainDirection.x + trainDirection.z * trainDirection.z);
        float trainPitch = atan2(-trainDirection.y, horizontalLength);

        float trainRoll = 0.0f;
        if (dist > 2.0f && dist < totalLength - 2.0f) {
            glm::vec3 leftPos = path.Evaluate(dist - 1.0f);
            glm::vec3 rightPos = path.Evaluate(dist + 1.0f);

            glm::vec3 bankingDirection = glm::cross(trainDirection, glm::vec3(0, 1, 0));
            float bankingIntensity = glm::dot(bankingDirection, glm::normalize(rightPos - leftPos));

            trainRoll = bankingIntensity * 0.15f;
        }

        glm::vec3 trainEulerRotation = glm::vec3(trainPitch, trainYaw, trainRoll);

        glm::vec3 trackRight = glm::normalize(glm::cross(trainDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        float lateralOffset = -1.3f;

        glm::vec3 adjustedTrainPos = trainPos + (trackRight * lateralOffset);
        adjustedTrainPos.y += 0.15f;

        model.SetPosition(adjustedTrainPos);
        model.SetRotation(trainEulerRotation);

        float frontOffset = 3.0f;
        frontPosition = adjustedTrainPos + trainDirection * frontOffset;
    }
}

void Train::Draw(Shader& shader) {
    model.Draw(shader);
}

glm::vec3 Train::GetFrontPosition() const
{
    return frontPosition;
}
