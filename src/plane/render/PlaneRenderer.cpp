#include "PlaneRenderer.h"
#include "../app/Plane.h"

#include <glm/gtc/matrix_transform.hpp>

namespace plane::render
{
    void PlaneRenderer::Draw(app::Plane& plane, Shader& shader, const core::PlaneState& planeState) const
    {
        glm::mat4 base = glm::mat4(1.0f);
        base = glm::translate(base, planeState.position);
        base = glm::rotate(base, glm::radians(planeState.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        base = glm::rotate(base, glm::radians(planeState.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        base = glm::rotate(base, glm::radians(planeState.roll), glm::vec3(0.0f, 0.0f, 1.0f));
        base = glm::scale(base, glm::vec3(0.006f, 0.006f, 0.006f));

        plane.Draw(shader, base);
    }
}

