#include "GameScene.h"
#include <numbers>

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	auto& ctx = GameContext::GetInstance();
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	camera_ = std::make_unique<Camera>();
	camera_->transform_.rotate = { 0,0,0 };
	camera_->transform_.translate = { 0,0,-cameraDistance_ };

	suzanne_ = ctx.Asset().LoadModel("Resources/Debug", "suzanne.obj");
	suzanne_->SetRotate({ 0, float(std::numbers::pi), 0 });
}

void GameScene::Update() {
	debugCamera_->Update();
	camera_->Update(debugCamera_.get());
}

void GameScene::Draw() {
	auto& ctx = GameContext::GetInstance();
	ctx.Render().DrawModel(suzanne_.get(), camera_.get());
}