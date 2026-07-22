#include "Passive.h"

Passive::Passive() {
}

Passive::Passive(Sprite* sprite) {
	sprite_ = sprite;
}

void Passive::Draw() {
	GameContext::GetInstance().Render().DrawSprite(sprite_);
}
