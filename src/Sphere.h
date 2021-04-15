#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "Object.h"

class Sphere : public Object {
    public:
    Sphere(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV);
    void draw(const std::shared_ptr<Program> prog);
};

#endif