#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>

#include "MatrixStack.h"
#include "Material.h"
#include "Shape.h"
#include "Program.h"

class Object {
protected:
    void sendGPU(const std::shared_ptr<Program> prog);
    virtual float pickColor();

    std::shared_ptr<Shape> shape;
    std::shared_ptr<MatrixStack> MV;
    Material mat;
    glm::vec3 pos;
    glm::vec3 posOffset;
    float yOffset;
    float scale;
    float rot;
    std::shared_ptr<double> t;
    float tOffset;
public:
    Object(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV);
    ~Object();
    virtual void draw(const std::shared_ptr<Program> prog);
    void addAnimation(std::shared_ptr<double> t);

};

#endif