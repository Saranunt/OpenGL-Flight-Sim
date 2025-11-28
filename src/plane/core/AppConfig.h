#pragma once

namespace plane::core
{
    // Central place for resolution constants so window + camera stay in sync.
    struct AppConfig
    {
        static constexpr unsigned int ScreenWidth = 1920;
        static constexpr unsigned int ScreenHeight = 1080;
    };
}

