#pragma once
#include <string>
class BaseScene;
class SceneManager;
class SceneEditor;
class SceneContext {
public:
	SceneContext(SceneManager* sceneManager, SceneEditor* sceneEditor);

	// シーン変更
	void SceneChange(std::string nextSceneName);

	BaseScene* GetCurrentScene();
	void SceneLoad(const std::string& path);
private:
	SceneManager* sceneManager_ = nullptr;
	SceneEditor* sceneEditor_ = nullptr;
};

