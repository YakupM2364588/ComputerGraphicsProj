#ifndef TRAIN_H
#define TRAIN_H
#include "model.h"
#include "bezierCurvesPath.h"

class Train {
public:
    Train(const std::string& modelPath, const std::string& mtlPath);
    void Update(float deltaTime, BezierCurvePath& path);
    void Draw(Shader& shader);
    glm::vec3 GetFrontPosition() const;

    Model model;
    float speed = 5.0f;

private:
    float m_accumulatedTime = 0.0f; //t
    glm::vec3 frontPosition;
};

#endif //TRAIN_H
