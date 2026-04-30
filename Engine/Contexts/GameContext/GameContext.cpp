#include "GameContext.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Io/AudioSystem/AudioSystem.h"
#include "Engine/Io/InputSystem/InputSystem.h"
#include "Engine/Asset/Manager/AssetManager/AssetManager.h"
#include "Engine/Object/LightManager/LightManager.h"

GameContext::GameContext(Renderer* renderer, AudioSystem* audio, InputSystem* input, AssetManager* assetManager, LightManager* lightManager, SceneManager* sceneManager) {
	asset_ = std::make_unique<AssetContext>(assetManager);
	audio_ = std::make_unique<AudioContext>(audio);
	input_ = std::make_unique<InputContext>(input);
	light_ = std::make_unique<LightContext>(lightManager);
	render_ = std::make_unique<RenderContext>(renderer, lightManager);
	scene_ = std::make_unique<SceneContext>(sceneManager);

	std::mt19937 randomEngine(randomDevice_());
	randomEngine_ = randomEngine;

	hwnd_ = input->GetHwnd();
}

Vector2 GameContext::GetWindowSize() const {
	RECT rect;
	GetClientRect(hwnd_, &rect);
	return { float(rect.right),float(rect.bottom) };
}

int GameContext::RandomInt(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine_);
}

float GameContext::RandomFloat(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine_);
}