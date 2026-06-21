#include "HitEffect.h"

void HitEffect::Initialize(const Vector3& pos, const Vector4& color) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	spark_ = asset.LoadModel("Resources/HitEffect", "hitSpark.obj");
	spark_->SetTranslate(pos);
	spark_->SetScale({});

	auto data = spark_->GetMaterial(0)->GetData();
	data.color = color;
	data.uvTransform.m[0][0] = 0.5f;
	data.uvTransform.m[1][1] = 0.5f;
	data.uvTransform.m[3][0] = 0.5f * ctx.RandomInt(0, 1);
	data.uvTransform.m[3][1] = 0.5f * ctx.RandomInt(0, 1);
	spark_->GetMaterial(1)->SetData(data);

	timer_ = std::make_unique<Timer>();
	timer_->Start(duration_);
}

void HitEffect::Update() {
	if (timer_ && timer_->IsActive()) {
		float rate = 1.0f - timer_->GetRemaining() / duration_;
		if (rate < peak_) {
			float x = rate / peak_;
			// easeOutExpo
			size_ = Lerp(0.0f, maxSize_, 1.0f - powf(2.0f, -10.0f * x));
		} else {
			float x = (rate - peak_) / (1.0f - peak_);
			// easeOutQuad
			size_ = Lerp(maxSize_, 0.0f, 1.0f - (1.0f - x) * (1.0f - x));
		}

		spark_->SetScale({ size_, size_, size_ });
		timer_->Update();
	}
}

void HitEffect::Draw(Camera* camera) {
	auto& render = GameContext::GetInstance().Render();

	Vector3 rotate = camera->transform_.rotate;
	rotate.x += float(std::numbers::pi);
	spark_->SetRotate(rotate);
	render.DrawModel(spark_.get());
}
