#pragma once


namespace plane::core
{
    struct PlaneState;
}

namespace plane::features::movement
{
    class BoosterSystem
    {
    public:
        void Update(core::PlaneState& planeState, float deltaTime) const;
    };
}

