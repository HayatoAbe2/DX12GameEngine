#include "GameScene.h"
#include <numbers>
#include "Character/Enemy/Enemies/Spiker.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& render = ctx.Render();
	auto& scene = ctx.Scene();

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	camera_ = std::make_unique<Camera>();

	LoadSound();

	// Skybox
	skybox_ = asset.LoadTexture("Resources/Skydome/skybox.dds");

	playerModel_ = asset.LoadModel("Resources/Debug/human", "walk.gltf");
	playerShadowModel_ = asset.LoadModel("Resources/Debug/human", "walk.gltf");
	MaterialData data = playerModel_->GetMaterial(0)->GetData();
	data.color = { 0.0f,0.0f,0.0f,1.0f };
	data.useEnvironmentMap = true;
	data.environmentIntensity = 1.0f;

	mapTile_ = std::make_unique<MapTile>();
	mapTile_->Initialize();

	// 武器マネージャー
	weaponManager_ = std::make_unique<WeaponManager>();
	weaponManager_->Initialize();

	// アイテムマネージャー
	itemManager_ = std::make_unique<ItemManager>();
	itemManager_->Initialize(weaponManager_.get());

	// エフェクト
	effectManager_ = std::make_unique<EffectManager>();
	effectManager_->Initialize();

	// 当たり判定
	collisionChecker_ = std::make_unique<CollisionChecker>();
	collisionChecker_->Initialize(effectManager_.get());

	// プレイヤー
	player_ = std::make_unique<Player>();
	player_->Initialize(std::move(playerModel_), std::move(playerShadowModel_), itemManager_.get());
	player_->SetWeapon(weaponManager_->GetWeapon(0));

	// 敵
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize();

	// 弾
	bulletManager_ = std::make_unique<BulletManager>();

	// フェード
	fade_ = std::make_unique<Fade>();

	resultBG_ = asset.LoadSprite("resources/Result/result.png");
	resultBG_->SetColor({ 1, 1, 1, 0.7f });

	resultCursor_ = asset.LoadSprite("resources/Result/cursor.png");
	resultCursor_->SetSize({ 48,56 });
	resultCursor_->SetColor({ 1, 1, 1, 0.7f });
	resultCursor_->SetPivot({ 0.5f,0.5f });

	// カメラ制御
	cameraController_ = std::make_unique<GameCameraController>(camera_.get());

	// フロア生成
	floorManager_ = std::make_unique<FloorManager>();

	// マップ判定
	mapCheck_ = std::make_unique<MapCheck>();
	mapTile_->UpdateMapChange(false, enableEditMode_);
	Reset();

	floorManager_->Initialize();

	isLoaded_ = false;

	player_->Update(mapCheck_.get(), camera_.get(), bulletManager_.get());

	// UI描画システム
	uiDrawer_ = std::make_unique<UIDrawer>();
	uiDrawer_->Initialize(player_.get(), floorManager_.get());

	Update();
}

void GameScene::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	auto& audio = ctx.Audio();
	auto& scene = ctx.Scene();

	switch (phase_) {
	case Phase::GAME:
		if (!enableEditMode_) {
			if (isPause_) {
				// ポーズ中
				if (input.keyboard.IsRelease(DIK_ESCAPE) || input.gamepad.IsPress(XINPUT_GAMEPAD_START)) {
					isPause_ = false;
				}

			} else {

				if (!fade_->IsActive() && fade_->GetPhase() == FadePhase::None) {
					// プレイヤー処理
 					player_->Update(mapCheck_.get(), camera_.get(), bulletManager_.get());

					// ゲームオーバー
					if (player_->IsDead()) {
						fade_->StartFadeOut(1.5f);
					}

					// ゴール判定
					Vector2 pos = { player_->GetTransform().translate.x,player_->GetTransform().translate.z };
					if (mapCheck_->IsGoal(pos, player_->GetRadius(), enemyManager_->GetEnemies().size() == 0)) {
						fade_->StartFadeOut();
						audio.SoundPlay(L"Resources/Sounds/SE/warp.mp3", false);
					}

					// 次の部屋移動判定
					for (auto& connector : floorManager_->GetConnector()) {
						if (CheckCollision(ToXZ(player_->GetTransform().translate), connector.collider)) {
							fade_->StartFadeOut();
							isRoomMoving_ = true;
							nextDirection_ = connector.direction;
						}
					}
				}

				// カメラ追従
				camera_->transform_.translate = player_->GetTransform().translate + Vector3{ 0,30,-19 };

				// 敵
				if (!enableEditMode_) {
					enemyManager_->Update(mapCheck_.get(), player_.get(), bulletManager_.get(), itemManager_.get());
					mapCheck_->SetCombat(enemyManager_->GetEnemies().size() != 0);
				}

				// 弾の処理
				bulletManager_->Update(mapCheck_.get(), effectManager_.get());
				for (const auto& bullet : bulletManager_->GetBullets()) {

					// 当たり判定
					collisionChecker_->Check(player_.get(), bullet, camera_.get(), bulletManager_.get());
					for (auto enemy : enemyManager_->GetEnemies()) {
						collisionChecker_->Check(enemy, bullet, camera_.get(), player_.get(), enemyManager_.get());
					}
				}
				// 敵とプレイヤー接触
				for (auto enemy : enemyManager_->GetEnemies()) {
					if (dynamic_cast<Spiker*>(enemy)) {
						collisionChecker_->Check(player_.get(), enemy, camera_.get());
					}
				}

				// アイテム
				itemManager_->Update(player_.get(), enemyManager_->GetEnemies().size() != 0);

				// マップ
				mapTile_->UpdateMapChange(!enemyManager_->GetEnemies().empty(), enableEditMode_);
				mapTile_->Update(enemyManager_->GetEnemies().size() == 0);
				mapCheck_->Update(mapTile_->GetMap());

				effectManager_->Update();

				uiDrawer_->Update();
			}

			// フェードアウト終了時
			if (fade_->GetPhase() == FadePhase::Faded) {
				if (player_->IsDead() || floorManager_->GetCurrentDepth() == 3) {
					// リザルト移行
					phase_ = Phase::RESULT;
					fade_->StartFadeIn();
				} else if (isRoomMoving_) {
					// 次の部屋
					MoveToNextRoom(nextDirection_);

					isRoomMoving_ = false;
				} else {
					// 次のフロア
					Reset();
					Initialize();

					fade_->StartFadeIn();
				}
			}
		}

		mapTile_->UpdateMapChange(!enemyManager_->GetEnemies().empty(), enableEditMode_);
		enemyManager_->SpawnCheck(player_->GetTransform().translate, mapCheck_.get());

		if (enableEditMode_) {
			isLoaded_ = false;
		} else {
			// 編集の反映,再配置
			if (!isLoaded_) {
				itemManager_->Load();
				enemyManager_->Load(weaponManager_.get());
			}

			isLoaded_ = true;
		}

		break;
	case Phase::RESULT:
		switch (fade_->GetPhase()) {
		case FadePhase::None:
		{
			if (resultTime_ == 0) {
				resultTimer_.Start(2.0f);
			}

			// リザルト
			if (resultArrowMove_ < 1.0f) {
				resultArrowMove_ = max(resultArrowMove_ + ctx.GetDeltatime() * 1, 1.0f);
			}

			resultTimer_.Update();
			resultTime_ += ctx.GetDeltatime();

			float left = ctx.GetRenderWindowSize().x / 10.0f;
			float width = ctx.GetRenderWindowSize().x - left * 2.0f;
			float endX = left + (float(floorManager_->GetCurrentDepth()) / float(floorManager_->kMaxDepth)) * width;

			float sinWave_ = sinf(10.0f * float(std::numbers::pi) * resultTime_ * 0.3f);
			resultCursor_->SetPosition({ endX * resultArrowMove_, 180 + sinWave_ * 10 });

			if (resultTimer_.IsFinished() &&
				(input.keyboard.IsRelease(DIK_SPACE) || input.gamepad.IsRelease(XINPUT_GAMEPAD_A))) {
				fade_->StartFadeOut();
				return;
			}
		}
		break;
		case FadePhase::Faded:
			scene.SceneChange("Game");
		}
	}
}

void GameScene::Draw() {
	BaseScene::Draw();
	fade_->Update();

	// カメラ行列更新
	camera_->Update(debugCamera_.get());
	debugCamera_->Update();

	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
	if (phase_ == Phase::RESULT) {
		render.AddPostEffect(PostEffectType::Grayscale);
		render.AddPostEffect(PostEffectType::Vignette);
	} else {
		render.AddPostEffect(PostEffectType::Outline);
	}

	if (fade_->GetPhase() != FadePhase::None) {
		render.AddPostEffect(PostEffectType::RadialBlur);
	}
	render.DrawSkybox(skybox_.get()); // パーティクルを後に描画したい

	player_->Draw(camera_.get());
	enemyManager_->Draw();
	bulletManager_->Draw(camera_.get());
	itemManager_->Draw();
	effectManager_->Draw(camera_.get());

	// ui
	if (!enableEditMode_) {
		uiDrawer_->Draw();
	}

	if (phase_ == Phase::RESULT) {
		resultBG_->SetSize(ctx.GetRenderWindowSize());
		render.DrawSprite(resultBG_.get());
		render.DrawSprite(resultCursor_.get());
	}

	fade_->Draw();

#ifdef USE_IMGUI
	ImGui::Begin("Weapon");
	ImGui::DragFloat3("cameraPos", &camera_->transform_.translate.x, 0.5f);
	if (ImGui::Button("Pistol")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 0);
	};
	if (ImGui::Button("AssaultRifle")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 1);
	};
	if (ImGui::Button("Shotgun")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 2);
	};
	if (ImGui::Button("Flame")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 3);
	};
	if (ImGui::Button("Wave")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 4);
	};
	if (ImGui::Button("Orbit")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 5);
	};
	if (ImGui::Button("Charge")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 6);
	};
	if (ImGui::Button("Accel")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 7);
	};
	if (ImGui::Button("Sniper")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 8);
	};
	if (ImGui::Button("Burst")) {
		itemManager_->SetSpawn(player_->GetTransform().translate, 9);
	};
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Pos", &camera_->transform_.translate.x, 0.5f);
	ImGui::End();
#endif
}

void GameScene::Reset() {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	scene.Reset();
	enemyManager_->Reset();
	itemManager_->Reset();
	bulletManager_->Reset();
	player_->Stop();

	// プレイヤー位置
	Vector3 pos = { floorManager_->GetStartPos().x, 0, floorManager_->GetStartPos().y };
	player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, pos });

	isLoaded_ = false;
	mapCheck_->Initialize(mapTile_->GetMap(), mapTile_->GetTileSize());
}

void GameScene::MoveToNextRoom(Direction direction) {
	Reset();
	fade_->StartFadeIn();

	// 次の部屋ロード
	floorManager_->LoadNextRoom(direction);
	// プレイヤー位置
	Vector3 pos = { floorManager_->GetStartPos().x, 0, floorManager_->GetStartPos().y };
	player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, pos });
}

void GameScene::LoadSound() {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	audio.SoundLoad(L"Resources/Sounds/SE/explosion.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/shoot.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/fire.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/floorClear.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/fall.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/warp.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/hit.mp3");

	audio.SoundLoad(L"Resources/Sounds/BGM/field.mp3");
	audio.SoundPlay(L"Resources/Sounds/BGM/field.mp3", true);
}