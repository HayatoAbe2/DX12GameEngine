#pragma once
#include <string>
#include "Engine/Math/Vector3/Vector3.h"
class BaseScene;
class SceneManager;
class SceneEditor;
class SceneContext {
public:
	SceneContext(SceneManager* sceneManager, SceneEditor* sceneEditor);

	// シーン変更
	void SceneChange(std::string nextSceneName);

	BaseScene* GetCurrentScene();
	void SceneLoad(const std::string& path, Vector3 offset = Vector3{0,0,0});
	void Reset();
private:
	SceneManager* sceneManager_ = nullptr;
	SceneEditor* sceneEditor_ = nullptr;
};

