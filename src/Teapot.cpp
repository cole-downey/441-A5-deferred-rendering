#include "Teapot.h"

using namespace std;

Teapot::Teapot(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV) :
    Bunny(_pos, _posOffset, _shape, _MV) {
}

void Teapot::draw(const shared_ptr<Program> prog) {
    // Set shader values
    glUniform3f(prog->getUniform("ka"), mat.ka.x, mat.ka.y, mat.ka.z);
    glUniform3f(prog->getUniform("kd"), mat.kd.x, mat.kd.y, mat.kd.z);
    glUniform3f(prog->getUniform("ks"), mat.ks.x, mat.ks.y, mat.ks.z);
    glUniform1f(prog->getUniform("s"), mat.s);

    // Apply model transforms
    MV->pushMatrix();
    MV->translate(pos);
    MV->scale(scale);
    MV->translate(posOffset); // make correct height
    MV->rotate(rot, 0, 1, 0);
    if (t != nullptr) MV->shear(0.0f, 0.0f, 0.0f, 0.5f * cos((float)*t + tOffset), 0.0f, 0.0f); // y shear in z direction

    // Draw	
    sendGPU(prog);
    MV->popMatrix();
}