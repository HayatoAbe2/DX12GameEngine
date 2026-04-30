#pragma once
#include <string>
class SceneManager;
class SceneContext {
public:
	SceneContext(SceneManager* sceneManager);

	// シーン変更
	void SceneChange(std::string nextSceneName);

private:
	SceneManager* sceneManager_ = nullptr;
};

