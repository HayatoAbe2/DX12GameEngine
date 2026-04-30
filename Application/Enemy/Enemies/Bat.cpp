#include "Bat.h"

void Bat::Attack(Weapon* weapon, BulletManager* bulletManager, Camera* camera) {
	if (attackCoolTimer_ <= 0) {
		// 射撃
		attackCoolTimer_ = weapon_->Shoot(model_->GetTransform().translate, attackDirection_, bulletManager, camera, true);

	} else {
		attackCoolTimer_--;
	}
}