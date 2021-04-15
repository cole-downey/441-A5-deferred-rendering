#include "Object.h"

using namespace std;


Object::Object(glm::vec3 _pos, glm::vec3 _posOffset, shared_ptr<Shape> _shape, shared_ptr<MatrixStack> _MV) :
    pos(_pos), posOffset(_posOffset),
    shape(_shape), MV(_MV), t(nullptr) {
    mat = Material({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, 200.0f);
    rot = 0.0f;
    scale = 1.0f;
}

Object::~Object() {
}

float Object::pickColor() {
    float color = (float)(rand() % 255);
    color /= 255.0f;
    return color;
}


void Object::draw(const shared_ptr<Program> prog) {
    // Set shader values
    glUniform3f(prog->getUniform("ka"), mat.ka.x, mat.ka.y, mat.ka.z);
    glUniform3f(prog->getUniform("kd"), mat.kd.x, mat.kd.y, mat.kd.z);
    glUniform3f(prog->getUniform("ks"), mat.ks.x, mat.ks.y, mat.ks.z);
    glUniform1f(prog->getUniform("s"), mat.s);

    // Apply model transforms
    MV->pushMatrix();
    MV->translate(pos);
    if (t == nullptr)
        MV->scale(scale);
    else
        MV->scale((float)cos(*t + tOffset) * scale * 0.10f + scale);
    MV->translate(posOffset); // make correct height
    MV->rotate(rot, 0, 1, 0);
    
    // Draw
    sendGPU(prog);
    MV->popMatrix();
}

void Object::sendGPU(const shared_ptr<Program> prog) {
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glm::mat4 it = glm::transpose(glm::inverse(MV->topMatrix()));
    glUniformMatrix4fv(prog->getUniform("IT"), 1, GL_FALSE, glm::value_ptr(it));
    shape->draw(prog);
}

void Object::addAnimation(shared_ptr<double> t) {
    this->t = t;
    tOffset = (float)rand() / (float)(RAND_MAX / (3.14f));
}
