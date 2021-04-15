#include "Teapot.h"

using namespace std;

Teapot::Teapot(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV) :
    Bunny(_pos, _posOffset, _shape, _MV) {
}

void Teapot::draw(const shared_ptr<Program> prog) {
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