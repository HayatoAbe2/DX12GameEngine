#include "SceneManager.h"
#include "Scene/SceneFactory/SceneFactory.h"

SceneManager::SceneManager() {
	sceneFactory_ = std::make_unique<SceneFactory>();
	currentScene_ = sceneFactory_->CreateScene("Game");
}

void SceneManager::Initialize() {
	currentScene_->Initialize();
}

void SceneManager::Update() {
	if (nextScene_) {
		currentScene_ = std::move(nextScene_);
		nextScene_ = nullptr;
	}

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
	nextScene_ = sceneFactory_->CreateScene(nextSceneName);
	nextScene_->Initialize();
}
