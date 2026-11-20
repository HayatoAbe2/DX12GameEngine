#include "Character.h"
#include "Reaction/Stun/CharacterStun.h"
#include "Reaction/Fall/CharacterFall.h"

void Character::Hit(float damage, const Vector2& from) {
}

void Character::Stun(float time) {
	// 反応状態を変更
	reaction_ = std::make_unique<CharacterStun>(time);
}

void Character::Fall() {
	// 反応状態を変更
	reaction_ = std::make_unique<CharacterFall>();
}
