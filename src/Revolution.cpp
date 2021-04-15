#include "Revolution.h"

using namespace std;

Revolution::Revolution(glm::vec3 _pos, glm::vec3 _posOffset, std::shared_ptr<Shape> _shape, std::shared_ptr<MatrixStack> _MV) :
    Object(_pos, _posOffset, _shape, _MV) {
    mat = Material(glm::vec3(0.0f), { pickColor(), pickColor(), pickColor() }, glm::vec3(1.0f), 10.0f);
    rot = 3.14f / 2;
    scale = 0.15f + (float)rand() / (float)(RAND_MAX / (0.6f - 0.15f));
}

void Revolution::draw(const shared_ptr<Program> prog) {
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
    MV->rotate(rot, 0, 0, 1);

    // Draw
    if (t == nullptr)
        glUniform1f(prog->getUniform("t"), 0.0f + (float)tOffset);
    else
        glUniform1f(prog->getUniform("t"), (float)(*t * (2.0 * (tOffset + 1.0) / 3.14)+ tOffset));
    sendGPU(prog);
    MV->popMatrix();
}