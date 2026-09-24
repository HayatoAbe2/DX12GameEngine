#include "SceneContext.h"
#include "Engine/Scene/SceneManager/SceneManager.h"
#include "Editor/SceneEditor/SceneEditor.h"

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

void SceneContext::SceneLoad(const std::string& path, Vector3 offset) {
	sceneEditor_->scene_ = sceneManager_->GetCurrentScene();
	sceneEditor_->Load(path, offset);
}

void SceneContext::Reset() {
	sceneEditor_->scene_->Clear();
}

std::vector<Model*> SceneContext::FindModelsByTag(const std::string& tag) {
    std::vector<Model*> result;

    for (auto& obj : sceneManager_->GetCurrentScene()->GetObjects()) {
        auto* model = dynamic_cast<Model*>(obj);
        if (model && model->tag == tag) {
            result.push_back(model);
        }
    }

    return result;
}

std::vector<InstancedModel*> SceneContext::FindInstancedModelsByTag(const std::string& tag) {
    std::vector<InstancedModel*> result;

    for (auto& obj : sceneManager_->GetCurrentScene()->GetObjects()) {
        auto* model = dynamic_cast<InstancedModel*>(obj);
        if (model && model->tag == tag) {
            result.push_back(model);
        }
    }

    return result;
}