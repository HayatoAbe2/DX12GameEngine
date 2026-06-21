#pragma once
#include "GameCommon.h"
#include "Engine/SceneObject/SceneObject.h"

/// <summary>
/// 各シーンの基底クラス
/// </summary>
class BaseScene {
public:

	virtual ~BaseScene() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw() = 0;

	// 登録オブジェクト取得
	std::vector<SceneObject*> GetObjects() { return sceneObjects_; }

protected:
	// カメラ
	std::unique_ptr<Camera> camera_ = nullptr;
	float cameraDistance_ = 20.0f;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;

	// オブジェクト
	std::vector<SceneObject*> sceneObjects_{};
};

