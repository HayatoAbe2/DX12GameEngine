#pragma once
#include "Engine/Scene/SceneFactoryBase/SceneFactoryBase.h"

/// <summary>
/// シーン管理
/// </summary>
class SceneManager {
public:
	SceneManager();

	// シーンの初期化
	void Initialize();
	// シーンの更新
	void Update();
	// シーンの描画
	void Draw();

	// シーン変更
	void SceneChange(std::string& nextSceneName);
	BaseScene* GetCurrentScene() { return currentScene_.get(); }

private:
	// 現在のシーン
	std::unique_ptr<BaseScene> currentScene_ = nullptr;

	// 基底シーンファクトリ(エンジン側)
	std::unique_ptr<SceneFactoryBase> sceneFactory_ = nullptr;
};

