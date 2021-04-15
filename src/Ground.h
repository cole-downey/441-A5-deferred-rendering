#ifndef _GROUND_H_
#define _GROUND_H_

#include "Object.h"

class Ground : public Object {
public:
    Ground(std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV) :
        Object(glm::vec3(0.0f), glm::vec3(0.0f), _shape, _MV) {
        scale = 1.0f;
        rot = 0.0f;
        rot = std::max(0.0f, rot);
        t = nullptr;
        //mat = Material({ 0.03f, 0.23f, 0.16f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1.0f);
        mat = Material({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 100.0f);
    };
};

#endif