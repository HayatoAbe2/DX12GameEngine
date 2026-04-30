#include "SceneContext.h"
#include "Engine/Scene/SceneManager/SceneManager.h"

SceneContext::SceneContext(SceneManager* sceneManager) {
	sceneManager_ = sceneManager;
}

void SceneContext::SceneChange(std::string nextSceneName) {
	sceneManager_->SceneChange(nextSceneName);
}
