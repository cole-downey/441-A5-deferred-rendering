#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include "Object.h"

class Light : public Object {
protected:
    glm::vec3* posCam; // position camera view
    glm::vec3* color;
    float pickColor();
    float r; // for polar coordinates
    float theta;
    bool polarEnabled = false;
public:
    Light(glm::vec3 _pos, glm::vec3* _posCam, glm::vec3* _color, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV);
    void addPolar(float _r, float _theta);
    void calcPolar();
    void refreshPosCam();
    void draw(const std::shared_ptr<Program> prog);
};



#endif