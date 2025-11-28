#pragma once

namespace plane::core
{
    // Stores frame timing so multiple systems see identical deltaTime.
    struct TimingState
    {
        float deltaTime { 0.0f };
        float lastFrame { 0.0f };
    };
}

