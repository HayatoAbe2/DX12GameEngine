#include "SceneManager.h"
#include "Scene/SceneFactory/SceneFactory.h"

SceneManager::SceneManager() {
	sceneFactory_ = std::make_unique<SceneFactory>();
}

void SceneManager::Initialize() {
	currentScene_ = sceneFactory_->CreateScene("Title");
	currentScene_->Initialize();
}

void SceneManager::Update() {
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::SceneChange(std::string& nextSceneName) {
	currentScene_ = sceneFactory_->CreateScene(nextSceneName);
	currentScene_->Initialize();
}
