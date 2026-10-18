#pragma once
#include "GameCommon.h"
#include "Objects/Character/Player/Player.h"

// インタラクト可能
class Interactable {
public:
    virtual ~Interactable() = default;

    virtual Vector2 GetPosition() const = 0;
    virtual bool CanInteract() const = 0;
    virtual void OnInteract(Player* player) = 0;
};