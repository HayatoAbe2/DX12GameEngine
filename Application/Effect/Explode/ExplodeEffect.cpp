#include "ExplodeEffect.h"

void ExplodeEffect::Initialize(const Vector3& pos, const Vector4& color, const float maxSize) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	dome_ = asset.LoadModel("Resources/HitEffect", "explode.obj");
	dome_->SetTranslate(pos);
	dome_->SetScale({});

	auto data = dome_->GetMaterial(0)->GetData();
	data.color = color;
	dome_->GetMaterial(1)->SetData(data);

	timer_ = std::make_unique<Timer>();
	timer_->Start(duration_);

	maxSize_ = maxSize;
}

void ExplodeEffect::Update() {
	if (timer_ && timer_->IsActive()) {
		float x = 1.0f - timer_->GetRemaining() / duration_;
		// easeOutd
		size_ = Lerp(0.0f, maxSize_, 1.0f - powf(2.0f, -10.0f * x));

		dome_->SetScale({ size_, size_, size_ });
		auto data = dome_->GetMaterial(1)->GetData();
		Matrix4x4 rot = MakeRotateYMatrix(x * float(std::numbers::pi) * 2.0f);
		data.uvTransform = Multiply(MakeIdentity4x4(), rot);
		if (x > alphaStart_) {
			data.color.w = 1.0f - (x - alphaStart_) / (1.0f - alphaStart_) - 0.2f;
		} else {
			data.color.w = 0.8f;
		}
		dome_->GetMaterial(1)->SetData(data);

		timer_->Update();
	}
}

void ExplodeEffect::Draw(Camera* camera) {
	auto& render = GameContext::GetInstance().Render();
	render.DrawModel(dome_.get());
}
