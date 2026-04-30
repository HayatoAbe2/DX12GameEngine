#pragma once
#include "Engine/Contexts/AssetContext/AssetContext.h"
#include "Engine/Contexts/AudioContext/AudioContext.h"
#include "Engine/Contexts/InputContext/InputContext.h"
#include "Engine/Contexts/LightContext/LightContext.h"
#include "Engine/Contexts/RenderContext/RenderContext.h"
#include "Engine/Contexts/SceneContext/SceneContext.h"
#include <random>
#include <memory>

class Renderer;
class AudioSystem;
class InputSystem;
class AssetManager;
class LightManager;

// 各Contextのまとめ役
class GameContext {
public:
	static GameContext& GetInstance() {
		assert(instance);
		return *instance;
	}

	// 最初にセットする。仮に複数要る場合これで変更
	static void Set(GameContext* context) {
		instance = context;
	}

	// コンストラクタ
	GameContext(Renderer* renderer, AudioSystem* audio, InputSystem* input, AssetManager* assetManager, LightManager* lightManager, SceneManager* sceneManager);

	// モデルなどの読み込み
	AssetContext& Asset() { return *asset_; }

	// 音声再生
	AudioContext& Audio() { return *audio_; }

	// 入力
	InputContext& Input() { return *input_; }

	// ライト管理
	LightContext& Light() { return *light_; }

	// 描画
	RenderContext& Render() { return *render_; }

	// シーン管理
	SceneContext& Scene() { return *scene_; }

	///
	/// ウィンドウ情報
	///

	Vector2 GetWindowSize() const;

	/// 
	/// 乱数
	///

	int RandomInt(int min, int max);
	float RandomFloat(float min, float max);

private:
	inline static GameContext* instance = nullptr;

	std::unique_ptr<AssetContext> asset_ = nullptr;
	std::unique_ptr<AudioContext> audio_ = nullptr;
	std::unique_ptr<InputContext> input_ = nullptr;
	std::unique_ptr<LightContext> light_ = nullptr;
	std::unique_ptr<RenderContext> render_ = nullptr;
	std::unique_ptr<SceneContext> scene_ = nullptr;

	// ランダム関連
	std::random_device randomDevice_;
	std::mt19937 randomEngine_;

	// ウィンドウサイズ確認用
	HWND hwnd_;
};