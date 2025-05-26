//
// Created by p0l on 5/26/25.
//

#ifndef TRAIN_H
#define TRAIN_H
#pragma once
#include "model.h"
#include "bezierCurvesPath.h"

class Train {
public:
    Train(const std::string& modelPath, const std::string& mtlPath);
    void Update(float currentTime, BezierCurvePath& path);
    void Draw(Shader& shader);
    void SetTimeOffset(float offset);

private:
    Model model;
    float timeOffset = 0.0f;
    const float speed = 5.0f;
};

#endif //TRAIN_H
