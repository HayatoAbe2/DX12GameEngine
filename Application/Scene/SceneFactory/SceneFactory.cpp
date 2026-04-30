#include "SceneFactory.h"
#include "Scene/TitleScene/TitleScene.h"
#include "Scene/GameScene/GameScene.h"

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName) {
	std::unique_ptr<BaseScene> newScene = nullptr;

	// シーン作成
	if (sceneName == "Title") {
		newScene = std::make_unique<TitleScene>();
	} else if (sceneName == "Game") {
		newScene = std::make_unique<GameScene>();
	}

	return std::move(newScene);
}
