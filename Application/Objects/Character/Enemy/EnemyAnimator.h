#pragma once
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include "Engine/Asset/Resource/Animation.h"
#include "Objects/Character/Enemy/EnemyData.h"
#include "Timer/Timer.h"

/// <summary>
/// 敵のアニメーション再生の管理
/// </summary>
class EnemyAnimator {
public:
	using Animations = std::array<std::shared_ptr<Animation>, static_cast<int>(EnemyAnimations::Count)>;

	// 再生処理
	using ApplyFunc = std::function<void(const std::shared_ptr<Animation>&, bool blend)>;

	void Initialize(const Animations& animations, ApplyFunc apply) {
		animations_ = animations;
		apply_ = std::move(apply);
		current_ = EnemyAnimations::Idle;
		timer_.Reset();
		isDying_ = false;
	}

	// 再生する。設定されていないアニメーションは何もしない
	void Play(EnemyAnimations type) {
		const std::shared_ptr<Animation>& animation = animations_[static_cast<int>(type)];
		if (!animation || !apply_ || isDying_) { return; }

		// 通常アニメーションがあるときだけ滑らかにつなぐ
		apply_(animation, animations_[static_cast<int>(EnemyAnimations::Idle)] != nullptr);
		current_ = type;

		if (type == EnemyAnimations::Idle) {
			timer_.Reset();
			return;
		}

		// 速度を変えたあとの、実際の再生時間だけ待つ
		timer_.Start((std::max)(animation->GetPlayDuration(), 0.01f));
		if (type == EnemyAnimations::Dead) { isDying_ = true; }
	}

	// 毎フレーム呼ぶ
	void Update() {
		if (timer_.GetState() != TimerState::Running) { return; }
		timer_.Update();
		if (timer_.GetState() != TimerState::Finished) { return; }
		timer_.Reset();

		if (current_ == EnemyAnimations::Dead) {
			isDying_ = false;   // 死亡アニメーションが終わった(最後のポーズのまま)
		} else {
			Play(EnemyAnimations::Idle);   // 攻撃・被弾が終わったら通常へ
		}
	}

	// 死亡アニメーションの再生中
	bool IsDying() const { return isDying_; }

	EnemyAnimations GetCurrent() const { return current_; }
	bool Has(EnemyAnimations type) const { return animations_[static_cast<int>(type)] != nullptr; }

private:
	Animations animations_{};
	ApplyFunc apply_;
	EnemyAnimations current_ = EnemyAnimations::Idle;
	Timer timer_;
	bool isDying_ = false;
};
