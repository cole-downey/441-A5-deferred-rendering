#ifndef _REVOLUTION_H_
#define _REVOLUTION_H_

#include "Object.h"

class Revolution : public Object {
    public:
    Revolution(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV);
    void draw(const std::shared_ptr<Program> prog);
};

#endif