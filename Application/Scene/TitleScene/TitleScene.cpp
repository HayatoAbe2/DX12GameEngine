#include "TitleScene.h"

TitleScene::~TitleScene() {
}

void TitleScene::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	audio.SoundLoad(L"Resources/Sounds/SE/press.mp3");

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	camera_ = std::make_unique<Camera>();
	camera_->transform_.rotate = { 0,0,0 };

	// 天球
	skydome_ = asset.LoadModel("Resources/Skydome", "skydome.obj", false);
	
	// フェード
	fade_ = asset.LoadSprite("resources/Debug/white1x1.png");
	fade_->SetSize(ctx.GetWindowSize() + Vector2{ 20,80 });
	fade_->SetColor({ 1.0f,1.0f,1.0f,0.0f });

	control_ = asset.LoadSprite("Resources/Control/gamestart.png");
	control_->SetSize({ 270,50 });
	control_->SetPosition({ 640 - 135,710 - 200 });

	logo_ = asset.LoadSprite("Resources/Control/title.png");
	logo_->SetSize({ 610,150 });
	logo_->SetPosition({ 640 - 305,700 - 500 });
}

void TitleScene::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& input = ctx.Input();
	auto& scene = ctx.Scene();

	Vector3 rotate = skydome_->GetTransform().rotate;
	skydome_->SetRotate(rotate);

	camera_->Update(debugCamera_.get());

	debugCamera_->Update();

	if (input.IsTriggerLeftClick() && !isFadeIn_ && !isFadeOut_) {
		isFadeOut_ = true;
		fadeTimer_ = 0;
		audio.SoundPlay(L"Resources/Sounds/SE/press.mp3", false);
	}

	if (isFadeIn_) {
		fadeTimer_++;
		fade_->SetColor({ 1.0f,1.0f,1.0f,1.0f - (float)fadeTimer_ / (float)kMaxFadeinTimer_ });
		if (fadeTimer_ >= kMaxFadeinTimer_) {
			isFadeIn_ = false;
			fadeTimer_ = kMaxFadeoutTimer_;
		}
	} else if (isFadeOut_) {
		fadeTimer_++;
		fade_->SetColor({ 1.0f,1.0f,1.0f,(float)fadeTimer_ / (float)kMaxFadeoutTimer_ });
		if (fadeTimer_ >= kMaxFadeoutTimer_) {
			isFadeOut_ = false;

			// ゲームオーバーまたはクリア
			scene.SceneChange("Game");
		}
	}
}

void TitleScene::Draw() {
	auto& render = GameContext::GetInstance().Render();

	render.DrawModel(skydome_.get(), camera_.get());
	render.DrawSprite(control_.get());
	render.DrawSprite(logo_.get());
	render.DrawSprite(fade_.get());
}
