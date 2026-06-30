#pragma once
#include <string>
class BaseScene;
class SceneManager;
class SceneContext {
public:
	SceneContext(SceneManager* sceneManager);

	// シーン変更
	void SceneChange(std::string nextSceneName);

	BaseScene* GetCurrentScene();
private:
	SceneManager* sceneManager_ = nullptr;
};

