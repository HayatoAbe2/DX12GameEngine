#include "SceneManager.h"
#include "Engine/Scene/BaseScene.h"
#include "Scene/GameScene/GameScene.h"

SceneManager::SceneManager(GameContext* context) {
	gameContext_ = context;
}

void SceneManager::Initialize() {
	currentScene_ = std::make_unique<GameScene>();
	currentSceneType_ = Scene::kGame;
	currentScene_->Initialize();
}

void SceneManager::Update() {
	if (currentScene_) {
		currentScene_->Update();
		if (currentScene_->IsFinished()) {
			if (currentSceneType_ == Scene::kTitle) {
				// シーン切り替え
				currentScene_ = std::make_unique<GameScene>();
				currentSceneType_ = Scene::kGame;
			} else if (currentSceneType_ == Scene::kGame) {
	
			}

			currentScene_->Initialize();
			currentScene_->Update();
		}
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::Finalize() {
}
