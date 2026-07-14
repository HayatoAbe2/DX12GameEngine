#include "SceneContext.h"
#include "Engine/Scene/SceneManager/SceneManager.h"
#include "Engine/Editor/Scene/SceneEditor/SceneEditor.h"

SceneContext::SceneContext(SceneManager* sceneManager, SceneEditor* sceneEditor) {
	sceneManager_ = sceneManager;
	sceneEditor_ = sceneEditor;
}

void SceneContext::SceneChange(std::string nextSceneName) {
	sceneManager_->SceneChange(nextSceneName);
}

BaseScene* SceneContext::GetCurrentScene() {
	return sceneManager_->GetCurrentScene();
}

void SceneContext::SceneLoad(const std::string& path) {
	sceneEditor_->Load(path);
}
