#pragma once
#include "Objects/Character/Reaction/CharacterReaction.h"

/// <summary>
/// 行動不能
/// </summary>
class CharacterStun : public CharacterReaction {
public:
	CharacterStun(float time) { timer_.Start(time); }
	bool Update(Character* self) override;
private:
};

