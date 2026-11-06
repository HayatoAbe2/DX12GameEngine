#pragma once
#include "GameCommon.h"
#include "Objects/Character/Player/Player.h"

/// <summary>
/// インタラクト可能
/// </summary>
class Interactable {
public:
    virtual ~Interactable() = default;

    virtual Vector2 GetPosition() const = 0;
    virtual bool CanInteract() const = 0;
    virtual void OnInteract(Player* player) = 0;
};