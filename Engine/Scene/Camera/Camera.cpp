#include "Camera.h"
#include "Engine/Scene/DebugCamera/DebugCamera.h"
#include "Engine/Contexts/GameContext/GameContext.h"

void Camera::Update(DebugCamera* debugCamera) {
	auto& ctx = GameContext::GetInstance();
	Transform cameraTransform = transform_;

	// シェイク
	if (shakeFrame_ > 0) {
		float t = static_cast<float>(shakeFrame_) / shakeEndFrame_;
		float amp = amplitude_ * t;

		cameraTransform.translate.x += ctx.RandomFloat(-amp, amp);
		cameraTransform.translate.y += ctx.RandomFloat(-amp, amp);
		cameraTransform.translate.z += ctx.RandomFloat(-amp, amp);

		--shakeFrame_;
	}

	// デバッグカメラがONならそちらを使う
	if (debugCamera && debugCamera->IsEnable()) {
		// デバッグカメラのビュー行列を使う
		viewMatrix_ = debugCamera->GetViewMatrix();
	} else {
		// 通常カメラのビュー
		viewMatrix_ = Inverse(MakeAffineMatrix(cameraTransform));
	}

	Vector2 windowSize = ctx.GetWindowSize();
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, windowSize.x / windowSize.y, 0.1f, 100.0f);
}

void Camera::StartShake(float amplitude, int frame) {
	amplitude_ = amplitude;
	shakeFrame_ = frame;
	shakeEndFrame_ = frame;
}
