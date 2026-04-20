#include "App.h"
#include "Engine/App/Window.h"
#include "Engine/Io/DumpExporter/DumpExporter.h"
#include "Engine/Io/Logger/Logger.h"
#include "Engine/Io/AudioSystem/AudioSystem.h"
#include "Engine/Io/InputSystem/InputSystem.h"
#include "Engine/Asset/Manager/AssetManager/AssetManager.h"
#include "Engine/Object/LightManager.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Contexts/GameContext/GameContext.h"

#include <format>
#include <cassert>

void App::Initialize() {
	// リソースリーク確認(最初に作る)
	leakCheck_ = std::make_unique<D3DResourceLeakChecker>(); // 最後に解放する

	// COMの初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// ダンプ作成機能
	dumpExporter_ = std::make_unique<DumpExporter>();
	// クラッシュ時ダンプファイルに記録する
	SetUnhandledExceptionFilter(dumpExporter_->ExportDump);

	// ロガー
	logger_ = std::make_unique<Logger>();

	// ウィンドウの生成
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;
	window_ = std::make_unique<Window>();
	window_->Initialize(kClientWidth, kClientHeight);
	logger_->Log(logger_->GetStream(), std::format("[Window] Initialization complete.\n"));

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// XAudio2
	audio_ = std::make_unique<AudioSystem>();
	audio_->Initialize();
	assert(&audio_);
	logger_->Log(logger_->GetStream(), std::format("[AudioSystem] Initialization complete.\n"));

	// DirectInputの初期化
	input_ = std::make_unique<InputSystem>(window_->GetInstance(), window_->GetHwnd());
	assert(&input_);
	logger_->Log(logger_->GetStream(), std::format("[Input] Initialization complete.\n"));

	// DirectX準備
	dxContext_ = std::make_unique<DirectXContext>();
	dxContext_->Initialize(kClientWidth, kClientHeight, window_->GetHwnd(), logger_.get());

	// 描画クラス
	renderer_ = std::make_unique<Renderer>();
	renderer_->Initialize(dxContext_.get());
	logger_->Log(logger_->GetStream(), std::format("[Renderer] Initialization complete.\n"));

	// リソース
	assetManager_ = std::make_unique<AssetManager>(dxContext_.get(), logger_.get());
	logger_->Log(logger_->GetStream(), std::format("[AssetManager] Initialization complete.\n"));

	// ライト
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(dxContext_->GetBufferManager());

	// コンテキスト
	gameContext_ = std::make_unique<GameContext>(renderer_.get(), audio_.get(), input_.get(), assetManager_.get(), lightManager_.get());
	gameContext_->Set(gameContext_.get());

	// シーンマネージャー
	sceneManager_ = std::make_unique<SceneManager>(gameContext_.get());
	sceneManager_->Initialize();
	logger_->Log(logger_->GetStream(), std::format("[SceneManager] Initialization complete.\n"));
}

void App::Run() {

	//-------------------------------------------------
	// メインループ
	//-------------------------------------------------

	MSG msg{};

	// ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {

			// キーボード・マウス入力の更新
			input_->Update();

			//-------------------------------------------------
			// ゲームの処理
			//-------------------------------------------------

			// 更新処理
			sceneManager_->Update();

			// 描画開始時に呼ぶ
			renderer_->BeginFrame();

			// 描画処理
			sceneManager_->Draw();

			// 描画終了時に呼ぶ
			renderer_->EndFrame();
		}
	}
}

void App::Finalize() {
	{
		// シーンマネージャー
		sceneManager_->Finalize();
		sceneManager_.reset();

		// 入力
		input_.reset();
		logger_->Log(logger_->GetStream(), std::format("[Input] Shutdown complete.\n"));

		lightManager_.reset();
		logger_->Log(logger_->GetStream(), std::format("[LightManager] Shutdown complete.\n"));

		assetManager_.reset();
		logger_->Log(logger_->GetStream(), std::format("[ResourceManager] Shutdown complete.\n"));

		gameContext_.reset();

		// AudioSystem
		audio_->StopAll();
		audio_->Finalize();
		audio_.reset();
		logger_->Log(logger_->GetStream(), std::format("[AudioSystem] Shutdown complete.\n"));

		// 描画クラス実体解放
		renderer_.reset();
		logger_->Log(logger_->GetStream(), std::format("[Renderer] Shutdown complete.\n"));

		// dx
		dxContext_->Finalize();
		dxContext_.reset();

		// ウィンドウ終了
		CloseWindow(window_->GetHwnd());
		window_.reset();
		logger_->Log(logger_->GetStream(), std::format("[Window] Closed.\n"));

		CoUninitialize();

		// ログ
		logger_.reset();

		// ダンプ作成クラス
		dumpExporter_.reset();

	}
	// 解放されていないリソースがあれば警告、停止
	leakCheck_.reset();
}