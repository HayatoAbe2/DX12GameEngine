#pragma once
#include "Objects/Character/Reaction/CharacterReaction.h"

constexpr float kCharacterFallSpeed = 0.7f;
constexpr float kCharacterFallTime = 0.5f;

/// <summary>
/// 落下中
/// </summary>
class CharacterFall : public CharacterReaction {
public:
	CharacterFall() { timer_.Start(kCharacterFallTime); }
	bool Update(Character* self) override;
	CharacterReactionState GetCurrentState() const override { return CharacterReactionState::Fall; }
};
