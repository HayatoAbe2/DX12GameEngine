#include "Bat.h"

void Bat::Attack(BulletManager* bulletManager, const Vector3& dir, Camera* camera) {
	attackCoolTimer_->Start(weapons_[0]->Trigger(model_->GetTransform().translate, dir, bulletManager, camera, this));
	isAttacking_ = false;
}