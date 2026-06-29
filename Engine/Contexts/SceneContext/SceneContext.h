#pragma once
#include <string>
class SceneManager;
class SceneContext {
public:
	SceneContext(SceneManager* sceneManager);

	// シーン変更
	void SceneChange(std::string nextSceneName);

	bool IsEditMode();
private:
	SceneManager* sceneManager_ = nullptr;
};

