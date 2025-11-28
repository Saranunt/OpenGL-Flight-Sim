#include "PlaneRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace plane::render
{
    void PlaneRenderer::Draw(Model& model, Shader& shader, const core::PlaneState& planeState) const
    {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, planeState.position);
        transform = glm::rotate(transform, glm::radians(planeState.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(planeState.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, glm::radians(planeState.roll), glm::vec3(0.0f, 0.0f, 1.0f));
        // Model was authored large, so we shrink it into the scene's units.
        transform = glm::scale(transform, glm::vec3(0.006f, 0.006f, 0.006f));

        shader.setMat4("model", transform);
        model.Draw(shader);
    }
}

