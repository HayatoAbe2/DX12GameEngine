#include "Knight.h"

void Knight::Attack(BulletManager* bulletManager, const Vector3& dir, Camera* camera) {	
	// 攻撃中
	if (comboCount_ < maxCombo_) {
		isAttacking_ = true;
		comboInterval_--;
		
		if (comboInterval_ <= 0) {
			// 射撃
			float time = weapons_[0]->Shoot(model_->GetTransform().translate, dir, bulletManager, camera, this);

			if (attackCoolTimer_->IsActive()) {
				attackCoolTimer_->AddTime(time);
			} else {
				attackCoolTimer_->Start(time);
			}

			comboCount_++;
			comboInterval_ = maxComboInterval_;
		}
	} else {
		// 終了後
		comboCount_ = 0;
		isAttacking_ = false;
	}
}