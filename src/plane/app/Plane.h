#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <learnopengl/model.h>
#include <learnopengl/shader_m.h>

namespace plane::app
{
    // Composite plane made of multiple sub-models (body, blade, flaps, tail).
    class Plane
    {
    public:
        enum class Part : int
        {
            Body = 0,
            Blade,
            FlapL,
            FlapR,
            Tail,
            Count
        };

        Plane();

        bool LoadModels();
        void InitializePartPositions();  // Set initial positions based on model structure

        // Draw all parts using a shared base transform; each part adds its own local transform.
        void Draw(Shader& shader, const glm::mat4& baseTransform);

        void SetPartTransform(Part part, const glm::mat4& transform);
        glm::mat4 GetPartTransform(Part part) const;
        void ResetPartTransform(Part part);
        void ResetAllTransforms();

        void SetPartPivot(Part part, const glm::vec3& pivotOffset);

        void RotatePart(Part part, const glm::vec3& axis, float radians);
        void TranslatePart(Part part, const glm::vec3& offset);

    private:
        std::string PartPath(Part part) const;

        std::unique_ptr<Model> models_[static_cast<int>(Part::Count)];
        glm::mat4 partTransforms_[static_cast<int>(Part::Count)];
        glm::vec3 partPivots_[static_cast<int>(Part::Count)];
    };
}
