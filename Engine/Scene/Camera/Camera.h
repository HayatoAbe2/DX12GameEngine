#pragma once
#include "Engine/Scene/DebugCamera/DebugCamera.h"
class Renderer;

class Camera {
public:

	Transform transform_ = { {1,1,1} };
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;

	/// <summary>
	/// view,projection行列の更新
	/// </summary>
	/// <param name="debugCamera">デバッグカメラ(なければnullptr)</param>
	void Update(DebugCamera* debugCamera = nullptr);

	void StartShake(float amplitude, int frame);
private:
	Vector3 shake_{};
	int shakeFrame_ = 0;
	int shakeEndFrame_;
	float amplitude_ = 0;
};