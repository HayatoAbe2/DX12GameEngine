#include "Chest.h"

Chest::Chest(Vector2 position) {
	auto& ctx = GameContext::GetInstance();
	auto asset = ctx.Asset();

	// モデル
	model_ = asset.LoadModel("Resources/Models/Chest", "Chest.glb");
	model_->SetRotate(Vector3(0, float(std::numbers::pi), 0));

	// アニメーション
	auto animation = asset.LoadAnimation("Resources/Models/Chest", "Chest.glb");
	anim_ = animation[kOpenAnimNumber_];
	model_->SetAnimation(anim_);

	appierTimer_.Start(kAppierTime);
	position_ = position;
}

void Chest::Update() {
	switch (phase_) {
	case ChestPhase::Closed:
	{
		// 降る
		appierTimer_.Update();
		float rate = 1.0f - (appierTimer_.GetRemaining() / kAppierTime);
		float t = rate * rate;
		height_ = kSpawnHeight * (1.0f - t);
		if (appierTimer_.IsFinished()) {
			phase_ = ChestPhase::PlayingAnimation;
			height_ = 0;
		}
	}
	break;
	case ChestPhase::PlayingAnimation:
		// 開くアニメーション再生
		model_->Update();

		if (model_->IsAnimationEnded()) {
			phase_ = ChestPhase::Opened;
			dyingTimer_.Start(kDyingTime);
		}
		break;
	case ChestPhase::Opened:
		break;
	case ChestPhase::Die:
	{
		// 消える
		dyingTimer_.Update();
		for (auto mat : model_->GetMaterials()) {
			auto data = mat->GetData();
			data.color.w = dyingTimer_.GetRemaining() / kDyingTime;
			mat->SetData(data);
		}
		if (dyingTimer_.IsFinished()) {
			isDead_ = true;
		}
	}
	}
}

void Chest::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto render = ctx.Render();
	model_->SetTranslate(Vector3(position_.x, height_, position_.y));
	render.DrawModel(model_.get());
}

bool Chest::CheckDrop() {
	if (phase_ == ChestPhase::Opened) {
		phase_ = ChestPhase::Die;
		return true;
	}
	return false;
}
