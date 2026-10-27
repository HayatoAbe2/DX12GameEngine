#include "TitleScene.h"

TitleScene::~TitleScene() {
}

void TitleScene::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();
	auto& scene = ctx.Scene();
	auto& render = ctx.Render();

	scene.SceneLoad("Resources/Debug/SceneEditor/startRoom1.json");
	audio.SoundLoad(L"Resources/Sounds/SE/press.mp3");

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	camera_ = std::make_unique<Camera>();
	camera_->transform_.translate = kCameraPos;
	camera_->transform_.rotate = kCameraRot;
	render.SetCamera(camera_.get());

	logo_ = asset.LoadSprite("Resources/Images/Text/logo_ai.png");
	logo_->SetPivot({ 0.5f,0.5f });
	logo_->SetSize(kLogoSize);
	logo_->SetPosition(ctx.GetRenderWindowSize() / 2.0f + kLogoOffset);
	control_ = asset.LoadSprite("Resources/Images/Text/pressAny.png");
	control_->SetPivot({ 0.5f,0.5f });
	control_->SetSize(kControlSize);
	control_->SetPosition(ctx.GetRenderWindowSize() / 2.0f + kControlOffset);

	// Skybox
	skybox_ = asset.LoadTexture("Resources/Debug/Sky/skybox.dds");
	
	// フェード
	fade_ = std::make_unique<Fade>();

	audio.SoundLoad(L"Resources/Sounds/BGM/title.mp3");
	audio.SoundPlay(L"Resources/Sounds/BGM/title.mp3", true, 0.7f);
}

void TitleScene::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& input = ctx.Input();
	auto& scene = ctx.Scene();

	camera_->Update(debugCamera_.get());

	debugCamera_->Update();

	fade_->Update();
	if ((input.keyboard.IsAnyTrigger() || input.gamepad.IsAnyTrigger()) &&
		fade_->GetPhase() == FadePhase::None) {
		fade_->StartFadeOut();
		audio.SoundPlay(L"Resources/Sounds/SE/press.mp3", false);
	}

	if (fade_->GetPhase() == FadePhase::Faded) {
			scene.SceneChange("Game");
	}
}

void TitleScene::Draw() {
	BaseScene::Draw();

	auto& render = GameContext::GetInstance().Render();
	render.AddPostEffect(PostEffectType::BoxFilter5x5);
	
	render.DrawSprite(logo_.get());
	render.DrawSprite(control_.get());
	render.DrawSkybox(skybox_.get());
	fade_->Draw();
}
