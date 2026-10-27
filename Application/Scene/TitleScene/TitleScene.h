#pragma once
#include "Engine/Scene/BaseScene/BaseScene.h"
#include "Timer/Timer.h"
#include "Fade/Fade.h"
#include <memory>

class Renderer;
class Camera;

class TitleScene : public BaseScene {
public:

	~TitleScene() override;

	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

private:
	std::shared_ptr<Texture> skybox_ = nullptr;

	std::unique_ptr<Fade> fade_ = nullptr;

	// スプライト
	std::unique_ptr<Sprite> logo_ = nullptr;
	Vector2 kLogoOffset = { 0, -80 };
	Vector2 kLogoSize = { 400, 237 };
	std::unique_ptr<Sprite> control_ = nullptr;
	Vector2 kControlOffset = { 0, 100 };
	Vector2 kControlSize = { 1300 * 0.5f, 250 * 0.5f };

	// カメラ
	std::unique_ptr<Camera> camera_ = nullptr;
	float cameraDistance_ = 20.0f;
	Vector3 kCameraPos = { 0,30,-19 };
	Vector3 kCameraRot = { 1.0f,0.0f,0.0f };

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;

	// タイマー
	Timer sceneChangeTimer_;
};
