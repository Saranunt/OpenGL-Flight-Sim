#include "SkeletalAnimationSystem.h"

#include <iostream>

namespace plane::features::animation
{
    void SkeletalAnimationSystem::Initialize()
    {
        // TODO: Integrate animation clips and bone hierarchy binding.
        std::cout << "[SkeletalAnimationSystem] Placeholder initialize.\n";
    }

    void SkeletalAnimationSystem::Update(float /*deltaTime*/)
    {
        // TODO: Blend and upload pose matrices to shaders.
    }
}

