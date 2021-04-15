#ifndef _TEAPOT_H_
#define _TEAPOT_H_

#include "Bunny.h"

class Teapot : public Bunny {
public:
    Teapot(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV);
    void draw(const std::shared_ptr<Program> prog);
};

#endif