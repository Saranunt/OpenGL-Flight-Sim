#include "MultiplayerManager.h"

#include <iostream>

namespace plane::features::multiplayer
{
    void MultiplayerManager::Initialize()
    {
        // TODO: Establish networking, state replication, and split-frame buffers.
        std::cout << "[MultiplayerManager] Placeholder initialize.\n";
    }

    void MultiplayerManager::Update(float /*deltaTime*/)
    {
        // TODO: Pump network events and drive multi-view rendering.
    }
}

