#include "DebugCamera.h"
#include "Engine/Contexts/GameContext/GameContext.h"

void DebugCamera::Initialize() {
	translation_ = { 0, 0, -50 };
	target_ = { 0, 0, 0 };
	distance_ = 50.0f;
	yaw_ = 0.0f;
	pitch_ = 0.0f;
	viewMatrix_ = MakeIdentity4x4();
	isEnable_ = false;

	auto& ctx = GameContext::GetInstance();
	input_ = &ctx.Input();
}

void DebugCamera::Update() {
	if (input_->keyboard.IsTrigger(DIK_RSHIFT)) {
		// デバッグカメラの切り替え
		isEnable_ = !isEnable_;
	}

	if (isEnable_) {
		ControlCamera();
		UpdateView();
	}
}

void DebugCamera::ControlCamera() {
	Vector3 forward = Normalize(target_ - translation_);
	Vector3 worldUp = { 0,1,0 };
	Vector3 right = Normalize(Cross(worldUp, forward));
	Vector3 up = Cross(forward, right);

	// shift+マウスホイール押し込み中,ドラッグで視点移動
	if (input_->keyboard.IsPress(DIK_LSHIFT) && input_->mouse.IsPress(MouseButton::Middle)) {

		float moveX = input_->mouse.GetMouseMove().x * kMoveSpeed_;
		float moveY = input_->mouse.GetMouseMove().y * kMoveSpeed_;
		float speed = distance_ * 0.02f; // 距離に比例して移動量変更
		
		target_ -= moveX * right * speed;
		target_ += moveY * up * speed;

	} else {

		// マウスホイール押し込み中,ドラッグで視点回転
		if (input_->mouse.IsPress(MouseButton::Middle)) {
			// マウスの移動量に回転速度を掛ける
			float deltaYaw = input_->mouse.GetMouseMove().x * kRotateSpeed_;   // マウスXでY軸回転（左右）
			float deltaPitch = input_->mouse.GetMouseMove().y * kRotateSpeed_; // マウスYでX軸回転（上下）

			yaw_ += deltaYaw;
			pitch_ += deltaPitch;

			const float limit = 1.55f;
			pitch_ = std::clamp(pitch_, -limit, limit);
		}
	}

	// マウスホイールでズームイン・ズームアウト
	float moveZ = input_->mouse.GetMouseMove().z * kMoveSpeed_;
	distance_ += -moveZ;

	// 中心地点計算
	translation_.x = target_.x - distance_ * cosf(pitch_) * sinf(yaw_);
	translation_.y = target_.y + distance_ * sinf(pitch_);
	translation_.z = target_.z - distance_ * cosf(pitch_) * cosf(yaw_);
}

void DebugCamera::UpdateView() {
	viewMatrix_ = MakeLookAtMatrix(translation_, target_, { 0.0f,1.0f,0.0f });
}