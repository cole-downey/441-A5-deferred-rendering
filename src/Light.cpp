#include "Light.h"

using namespace std;

Light::Light(glm::vec3 _pos, glm::vec3* _posCam, glm::vec3* _color, shared_ptr<Shape> _shape, shared_ptr<MatrixStack> _MV) :
    Object(_pos, glm::vec3(0.0f), _shape, _MV),
    posCam(_posCam), color(_color) {
    mat = Material({ pickColor(), pickColor(), pickColor() }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 1.0f);
    *color = mat.ka;
    rot = 0.0f;
    scale = 0.03125f;
}

float Light::pickColor() {
    // overidden to make light colors brighter than regular object diffuse
    float color = (float)(rand() % 100) + 155.0f;
    color /= 255.0f;
    return color;
}

void Light::addPolar(float _r, float _theta) {
    r = _r;
    theta = _theta;
    polarEnabled = true;
    calcPolar();
}

void Light::calcPolar() {
    if (!polarEnabled) return;
    if (t == nullptr) {
        pos.x = (float)cos(theta) * r;
        pos.z = (float)sin(theta) * r;
    } else {
        pos.x = (float)cos((theta + *t) * tOffset * 0.5f / max(r, 1.0f)) * r;
        pos.z = (float)sin((theta + *t) * tOffset * 0.5f / max(r, 1.0f)) * r;
    }
}
void Light::refreshPosCam() {
    // updates posCam array to be in camera space
    auto temp = glm::vec4(pos, 1.0f);
    temp = MV->topMatrix() * temp;
    *posCam = glm::vec3(temp);
}

void Light::draw(const shared_ptr<Program> prog) {
    // Apply model transforms
    calcPolar();
    MV->pushMatrix();
    MV->translate(pos);
    MV->scale(scale);
    MV->translate(posOffset); // make correct height
    MV->rotate(rot, 0, 1, 0);

    // Draw
    sendGPU(prog);
    MV->popMatrix();
}