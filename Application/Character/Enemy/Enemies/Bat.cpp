#include "Bat.h"

void Bat::Attack(BulletManager* bulletManager, const Vector2& dir) {
	attackCoolTimer_->Start(weapons_[0]->Trigger(model_->GetTransform().translate, dir, bulletManager, this));
	isAttacking_ = false;
}