#include "BoosterSystem.h"

#include "core/PlaneState.h"

#include <algorithm>
#include <cmath>

namespace plane::features::movement
{
    namespace
    {
        float ExponentialApproach(float current, float target, float deltaTime, float timeConstantSeconds)
        {
            const float safeDt = std::max(0.0f, deltaTime);
            const float tau = std::max(0.001f, timeConstantSeconds);
            const float alpha = 1.0f - std::exp(-safeDt / tau);
            return current + (target - current) * alpha;
        }
    }

    void BoosterSystem::Update(core::PlaneState& planeState, float deltaTime) const
    {
        if (!planeState.isAlive)
        {
            planeState.isBoosting = false;
            planeState.speed = planeState.baseSpeed;
            return;
        }

        const float maxFuel = std::max(0.0f, planeState.boosterMaxFuelSeconds);
        const float rechargeSeconds = std::max(0.001f, planeState.boosterRechargeSeconds);

        planeState.boosterFuelSeconds = std::clamp(planeState.boosterFuelSeconds, 0.0f, maxFuel);

        bool canBoost = !planeState.boosterExhausted && (maxFuel > 0.0f) && (planeState.boosterFuelSeconds > 0.0f);
        bool shouldBoost = planeState.boostHeld && canBoost;

        if (shouldBoost)
        {
            planeState.isBoosting = true;
            planeState.boosterFuelSeconds = std::max(0.0f, planeState.boosterFuelSeconds - std::max(0.0f, deltaTime));

            if (planeState.boosterFuelSeconds <= 0.0f)
            {
                planeState.boosterFuelSeconds = 0.0f;
                planeState.isBoosting = false;
                planeState.boosterExhausted = true;
            }
        }
        else
        {
            planeState.isBoosting = false;

            if (maxFuel > 0.0f && planeState.boosterFuelSeconds < maxFuel)
            {
                const float rechargeRate = maxFuel / rechargeSeconds;
                planeState.boosterFuelSeconds = std::min(maxFuel, planeState.boosterFuelSeconds + rechargeRate * std::max(0.0f, deltaTime));
            }

            if (planeState.boosterExhausted && planeState.boosterFuelSeconds >= maxFuel)
            {
                planeState.boosterFuelSeconds = maxFuel;
                planeState.boosterExhausted = false;
            }
        }

        const float multiplier = planeState.isBoosting ? std::max(1.0f, planeState.boosterSpeedMultiplier) : 1.0f;
        const float targetSpeed = std::max(0.0f, planeState.baseSpeed * multiplier);

        const float rampSeconds = planeState.isBoosting
            ? std::max(0.001f, planeState.boosterRampUpSeconds)
            : std::max(0.001f, planeState.boosterRampDownSeconds);

        planeState.speed = ExponentialApproach(planeState.speed, targetSpeed, deltaTime, rampSeconds);
    }
}
