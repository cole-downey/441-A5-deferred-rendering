#include "Sphere.h"

using namespace std;

Sphere::Sphere(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV) :
    Object(_pos, _posOffset, _shape, _MV) {
    mat = Material(glm::vec3(0.0f), { pickColor(), pickColor(), pickColor() }, glm::vec3(1.0f), 10.0f);
    rot = (float)rand();
    scale = 0.0625f + (float)rand() / (float)(RAND_MAX / (0.25f - 0.0625f));
}

void Sphere::draw(const shared_ptr<Program> prog) {
    // Set shader values
    glUniform3f(prog->getUniform("ka"), mat.ka.x, mat.ka.y, mat.ka.z);
    glUniform3f(prog->getUniform("kd"), mat.kd.x, mat.kd.y, mat.kd.z);
    glUniform3f(prog->getUniform("ks"), mat.ks.x, mat.ks.y, mat.ks.z);
    glUniform1f(prog->getUniform("s"), mat.s);

    // Apply model transforms
    MV->pushMatrix();
    MV->translate(pos);
    if (t == nullptr) {
        MV->scale(scale);
    } else {
        float yBounce = (0.25f * tOffset / 3.14f + 0.125f) * (0.5f * (float)sin(2 * 3.14f * (*t + 0.9f + tOffset) / 2.7f) + 0.5f);
        float sBounce = -0.25f * (0.5f * (float)cos(4 * 3.14f * (*t + 0.9f + tOffset) / 2.7f) + 0.5f) + 1.0f;
        MV->translate(0.0f, yBounce, 0.0f);
        MV->scale(sBounce * scale, scale, sBounce * scale);
    }
    MV->translate(posOffset); // make correct height
    MV->rotate(rot, 0, 1, 0);

    // Draw
    sendGPU(prog);
    MV->popMatrix();
}