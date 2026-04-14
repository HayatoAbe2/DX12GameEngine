#include "GameScene.h"
#include <numbers>

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize(context_);
	camera_ = std::make_unique<Camera>();
	camera_->transform_.rotate = { 0,0,0 };
	camera_->transform_.translate = { 0,0,-cameraDistance_ };

	suzanne_ = context_->LoadModel("Resources", "suzanne.obj");
	suzanne_->SetRotate({ 0, float(std::numbers::pi), 0 });
}

void GameScene::Update() {
	debugCamera_->Update();
	camera_->Update(context_, debugCamera_.get());
}

void GameScene::Draw() {
	context_->DrawModel(suzanne_.get(), camera_.get());
}