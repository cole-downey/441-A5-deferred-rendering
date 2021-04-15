#include "Bunny.h"

using namespace std;

Bunny::Bunny(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV) :
    Object(_pos, _posOffset, _shape, _MV) {
    mat = Material(glm::vec3(0.0f), { pickColor(), pickColor(), pickColor() }, glm::vec3(1.0f), 10.0f);
    rot = (float)rand();
    scale = 0.125f + (float)rand() / (float)(RAND_MAX / (0.5f - 0.125f));

}

void Bunny::draw(const shared_ptr<Program> prog) {
    // Apply model transforms
    MV->pushMatrix();
    MV->translate(pos);
    MV->scale(scale);
    MV->translate(posOffset); // make correct height
    if (t == nullptr)
        MV->rotate(rot, 0, 1, 0);
    else
        MV->rotate(rot + (float)*t * 0.5f, 0, 1, 0);

    // Draw	
    sendGPU(prog);
    MV->popMatrix();
}