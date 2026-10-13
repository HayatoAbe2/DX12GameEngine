#include "HeavyKnight.h"

void HeavyKnight::Attack(BulletManager* bulletManager, const Vector2& dir) {

	// 攻撃中
	if (comboCount_ < maxCombo_) {
		isAttacking_ = true;
		comboInterval_--;

		if (comboInterval_ <= 0) {
			// 射撃
			float time = currentWeapon_->Trigger(model_->GetTransform().translate, dir, bulletManager, this);

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

		// 武器を交換する
		if (weapons_.size() - 1 == weaponNum_) {
			// 0番目に戻る
			currentWeapon_ = weapons_[0].get();
			weaponNum_ = 0;
		} else {
			// 次の武器
			currentWeapon_ = weapons_[++weaponNum_].get();
		}
	}
}