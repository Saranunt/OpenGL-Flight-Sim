#include "Plane.h"

#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/filesystem.h>

namespace plane::app
{
    Plane::Plane()
    {
        ResetAllTransforms();
        for (auto& p : partPivots_)
        {
            p = glm::vec3(0.0f);
        }
        // Set default initial positions for all parts relative to plane origin.
        InitializePartPositions();
    }

    bool Plane::LoadModels()
    {
        bool ok = true;
        for (int i = 0; i < static_cast<int>(Part::Count); ++i)
        {
            const auto path = PartPath(static_cast<Part>(i));
            try
            {
                models_[i] = std::make_unique<Model>(FileSystem::getPath(path));
            }
            catch (...)
            {
                ok = false;
                models_[i].reset();
            }
        }
        return ok;
    }

    void Plane::InitializePartPositions()
    {
        // Scale factor from the plane model (0.006 in PlaneRenderer)
        const float scale = 100.0f;

        // Part positions relative to plane origin, scaled down
        // Blade: (-0.003595, -2.0067, -0.293755) * 0.006
        SetPartTransform(Part::Blade, glm::translate(glm::mat4(1.0f), glm::vec3(-0.003595f, -0.293755f, 2.0067f) * scale));

        // FlapL: (-2.95947, -0.515343, -0.358399) * 0.006 (mirrored Z)
        SetPartTransform(Part::FlapL, glm::translate(glm::mat4(1.0f), glm::vec3(2.95947f, 0.358399f,  0.515343f) * scale));

        // FlapR: (-2.95947, -0.515343, 0.358399) * 0.006
        SetPartTransform(Part::FlapR, glm::translate(glm::mat4(1.0f), glm::vec3(-2.95947f,  0.358399f,    0.515343f) * scale));

        // Tail: (-0.000055, 2.62764, -0.032654) * 0.006
        SetPartTransform(Part::Tail, glm::translate(glm::mat4(1.0f), glm::vec3(-0.000055f, -0.032654f, -2.62764f) * scale));

        // Body stays at origin (0, 0, 0)
        SetPartTransform(Part::Body, glm::mat4(1.0f));
    }
    void Plane::Draw(Shader& shader, const glm::mat4& baseTransform)
    {
        for (int i = 0; i < static_cast<int>(Part::Count); ++i)
        {
            if (!models_[i])
                continue;

            const glm::vec3 pivot = partPivots_[i];
            glm::mat4 pivotTranslate    = glm::translate(glm::mat4(1.0f),  pivot);
            glm::mat4 pivotTranslateInv = glm::translate(glm::mat4(1.0f), -pivot);

            // Apply pivot so rotations happen around the part's local origin.
            glm::mat4 model = baseTransform * pivotTranslate * partTransforms_[i] * pivotTranslateInv;
            shader.setMat4("model", model);
            models_[i]->Draw(shader);
        }
    }

    void Plane::SetPartTransform(Part part, const glm::mat4& transform)
    {
        partTransforms_[static_cast<int>(part)] = transform;
    }

    glm::mat4 Plane::GetPartTransform(Part part) const
    {
        return partTransforms_[static_cast<int>(part)];
    }

    void Plane::ResetPartTransform(Part part)
    {
        partTransforms_[static_cast<int>(part)] = glm::mat4(1.0f);
    }

    void Plane::ResetAllTransforms()
    {
        for (auto& t : partTransforms_)
        {
            t = glm::mat4(1.0f);
        }
    }

    void Plane::SetPartPivot(Part part, const glm::vec3& pivotOffset)
    {
        partPivots_[static_cast<int>(part)] = pivotOffset;
    }

    void Plane::RotatePart(Part part, const glm::vec3& axis, float radians)
    {
        const int idx = static_cast<int>(part);
        partTransforms_[idx] = glm::rotate(partTransforms_[idx], radians, axis);
    }

    void Plane::TranslatePart(Part part, const glm::vec3& offset)
    {
        const int idx = static_cast<int>(part);
        partTransforms_[idx] = glm::translate(partTransforms_[idx], offset);
    }

    std::string Plane::PartPath(Part part) const
    {
        switch (part)
        {
        case Part::Body:   return "resources/objects/plane2/plane_body.dae";
        case Part::Blade:  return "resources/objects/plane2/plane_blade.dae";
        case Part::FlapL:  return "resources/objects/plane2/plane_flap_L.dae";
        case Part::FlapR:  return "resources/objects/plane2/plane_flap_R.dae";
        case Part::Tail:   return "resources/objects/plane2/plane_tail.dae";
        default:           return "";
        }
    }
}
