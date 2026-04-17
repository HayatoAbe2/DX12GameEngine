#include "Camera.h"
#include "DebugCamera.h"
#include "Engine/Contexts/GameContext/GameContext.h"

void Camera::Update(DebugCamera* debugCamera) {
	// デバッグカメラがONならそちらを使う
	if (debugCamera && debugCamera->IsEnable()) {
		// デバッグカメラのビュー行列を使う
		viewMatrix_ = debugCamera->GetViewMatrix();
	} else {
		// 通常カメラのビュー
		viewMatrix_ = Inverse(MakeAffineMatrix(transform_));
	}

	auto& ctx = GameContext::GetInstance();
	Vector2 windowSize = ctx.GetWindowSize();
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, windowSize.x / windowSize.y, 0.1f, 100.0f);
}

void Camera::StartShake(float amplitude, int frame) {
	amplitude_ = amplitude;
	shakeFrame_ = frame;
	shakeEndFrame_ = frame;
}
