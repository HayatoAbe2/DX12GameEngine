#include "GameScene.h"
#include <numbers>
#include "Character/Enemy/Enemies/Spiker.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& asset = ctx.Asset();
	auto& render = ctx.Render();
	auto& scene = ctx.Scene();

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	camera_ = std::make_unique<Camera>();
	camera_->transform_.rotate = { 1.0f,0,0 };
	render.SetCamera(camera_.get());

	audio.SoundLoad(L"Resources/Sounds/SE/explosion.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/shoot.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/fire.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/floorClear.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/fall.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/warp.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/hit.mp3");

	// Skybox
	skybox_ = asset.LoadTexture("Resources/Skydome/skybox.dds");

	playerModel_ = asset.LoadModel("Resources/Debug/human", "walk.gltf");
	playerModel_->SetAnimation(asset.LoadAnimation("Resources/Debug/human", "walk.gltf"));
	playerShadowModel_ = asset.LoadModel("Resources/Debug/human", "walk.gltf");
	MaterialData data = playerModel_->GetMaterial(0)->GetData();
	data.color = { 0.0f,0.0f,0.0f,1.0f };
	data.useEnvironmentMap = true;
	data.environmentIntensity = 1.0f;

	// マップ
	std::unique_ptr<InstancedModel> wall = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 500);
	std::unique_ptr<InstancedModel> wallShadow = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 500);
	std::unique_ptr<InstancedModel> floor = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 900);
	std::unique_ptr<InstancedModel> barrier = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 500);
	std::unique_ptr<InstancedModel> enemySpawn = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 30);
	std::unique_ptr<InstancedModel> enemySpawnPoint = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 300);
	std::unique_ptr<InstancedModel> weaponSpawn = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 30);
	std::unique_ptr<InstancedModel> start = asset.LoadInstancedModel("Resources/Tiles", "sphere.obj", 1);
	std::unique_ptr<InstancedModel> goal = asset.LoadInstancedModel("Resources/Tiles", "sphere.obj", 1);

	auto matData = wall->GetMaterial(0)->GetData();
	matData.useEnvironmentMap = true;
	matData.environmentIntensity = 0.6f;
	wall->GetMaterial(0)->SetEnvironmentTexture(skybox_);
	wall->GetMaterial(0)->SetData(matData);
	wall->GetMaterial(1)->SetEnvironmentTexture(skybox_);
	wall->GetMaterial(1)->SetData(matData);

	matData = barrier->GetMaterial(0)->GetData();
	matData.color = { 0.3f, 0.3f, 1.0f, 0.5f };
	barrier->GetMaterial(0)->SetData(matData);
	barrier->GetMaterial(1)->SetData(matData);
	matData.color = { 1.0f, 0.3f, 0.3f, 0.5f };
	enemySpawn->GetMaterial(0)->SetData(matData);
	enemySpawn->GetMaterial(1)->SetData(matData);
	matData.color = { 0.3f, 1.0f, 0.3f, 0.5f };
	weaponSpawn->GetMaterial(0)->SetData(matData);
	weaponSpawn->GetMaterial(1)->SetData(matData);

	wall->tag = "wall";
	floor->tag = "floor";
	barrier->tag = "barrier";
	enemySpawn->tag = "enemySpawn";
	enemySpawnPoint->tag = "enemySpawnPoint";
	weaponSpawn->tag = "weaponSpawn";
	start->tag = "start";
	goal->tag = "goal";
	this->AddObject(std::move(wall));
	this->AddObject(std::move(floor));
	this->AddObject(std::move(barrier));
	this->AddObject(std::move(enemySpawn));
	this->AddObject(std::move(enemySpawnPoint));
	this->AddObject(std::move(weaponSpawn));
	this->AddObject(std::move(start));
	this->AddObject(std::move(goal));

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
	player_->Initialize(std::move(playerModel_), std::move(playerShadowModel_));
	player_->SetWeapon(weaponManager_->GetWeapon(0));

	// 敵
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize();

	// 弾
	bulletManager_ = std::make_unique<BulletManager>();

	// UI描画システム
	uiDrawer_ = std::make_unique<UIDrawer>();
	uiDrawer_->Initialize(player_.get());

	camera_->transform_.translate = player_->GetTransform().translate + Vector3{ 0,0,-cameraDistance_ };

	// フェード
	fade_ = asset.LoadSprite("resources/Debug/white1x1.png");
	fade_->SetSize(ctx.GetWindowSize() + Vector2{ 20,80 });
	fade_->SetColor({ 1.0f,1.0f,1.0f,1.0f });

	resultBG_ = asset.LoadSprite("resources/Result/result.png");
	resultBG_->SetSize(ctx.GetWindowSize() + Vector2{ 20,80 });
	resultBG_->SetColor({ 1, 1, 1, 0.7f });

	resultCursor_ = asset.LoadSprite("resources/Result/cursor.png");
	resultCursor_->SetSize({ 48,56 });
	resultCursor_->SetColor({ 1, 1, 1, 0.7f });

	// マップ判定
	mapCheck_ = std::make_unique<MapCheck>();
	mapTile_->UpdateMapChange(false, enableEditMode_);
	Reset();

#ifdef USE_IMGUI
	enableEditMode_ = true;
#endif
	isLoaded_ = false;
}

void GameScene::Update() {
	if (!enableEditMode_) {
		auto& ctx = GameContext::GetInstance();
		auto& input = ctx.Input();
		auto& audio = ctx.Audio();
		auto& scene = ctx.Scene();
		if (!isShowResult_) {
			if (isPause_) {
				// ポーズ中
				if (input.keyboard.IsRelease(DIK_ESCAPE) || input.gamepad.IsPress(XINPUT_GAMEPAD_START)) {
					isPause_ = false;
				}

			} else {
				// プレイヤー処理
				if (!isFadeOut_) {
					player_->Update(mapCheck_.get(), itemManager_.get(), camera_.get(), bulletManager_.get());
				}

				// ゲームオーバー
				if (player_->IsDead() && !isFadeOut_) {
					isFadeOut_ = true;
					fadeTimer_ = 0;
				}

				// ゴール判定
				Vector2 pos = { player_->GetTransform().translate.x,player_->GetTransform().translate.z };
				if (mapCheck_->IsGoal(pos, player_->GetRadius(), enemyManager_->GetEnemies().size() == 0) && !isFadeOut_) {
					isFadeOut_ = true;
					fadeTimer_ = 0;
					audio.SoundPlay(L"Resources/Sounds/SE/warp.mp3", false);
				}

				// カメラ追従
				camera_->transform_.translate = player_->GetTransform().translate + Vector3{ 0,30,-19 };

				// 敵
				if (!enableEditMode_) {
					enemyManager_->Update(mapCheck_.get(), player_.get(), bulletManager_.get());
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
				itemManager_->Update(player_.get());

				// マップ
				mapTile_->UpdateMapChange(!enemyManager_->GetEnemies().empty(), enableEditMode_);
				mapTile_->Update(enemyManager_->GetEnemies().size() == 0);
				mapCheck_->Update(mapTile_->GetMap());

				effectManager_->Update();

				uiDrawer_->Update();
			}

			if (isFadeIn_) {
				fadeTimer_++;
				fade_->SetColor({ 1.0f,1.0f,1.0f,1.0f - (float)fadeTimer_ / (float)kMaxFadeinTimer_ });
				if (fadeTimer_ >= kMaxFadeinTimer_) {
					isFadeIn_ = false;
					fadeTimer_ = 0;
				}
			} else if (isFadeOut_) {
				fadeTimer_++;
				fade_->SetColor({ 1.0f,1.0f,1.0f,(float)fadeTimer_ / (float)kMaxFadeoutTimer_ });
				if (fadeTimer_ >= kMaxFadeoutTimer_) {
					isFadeOut_ = false;

					if (player_->IsDead() || currentFloor_ == 3) {
						if (isShowResult_) {
							scene.SceneChange("Game");
						} else {
							// ゲームオーバーまたはクリア
							isShowResult_ = true;

							fadeTimer_ = 0;
							isFadeIn_ = true;
						}
					} else {
						fadeTimer_ = 0;
						isFadeIn_ = true;
						// 次のフロア
						currentFloor_++;
						Reset();
					}
				}

			} else {
			}

		} else {
			// リザルト
			if (resultArrowMove_ < 1.0f) {
				resultArrowMove_ += ctx.GetDeltatime() * 1;
			}

			resultTime_ += ctx.GetDeltatime();
			float endX = 0;
			switch (currentFloor_) {
			case 0:
				endX = 205;
				break;
			case 1:
				endX = 471;
				break;
			case 2:
				endX = 765;
				break;
			case 3:
				endX = 1033;
				break;
			case 4:
				endX = 1300;
				break;
			}
			float sinWave_ = sinf(10.0f * float(std::numbers::pi) * resultTime_ * 0.3f);
			resultCursor_->SetPosition({ endX * resultArrowMove_,180 + sinWave_ * 10 });

			if (input.keyboard.IsRelease(DIK_SPACE) || input.gamepad.IsRelease(XINPUT_GAMEPAD_A)) {
				isFadeOut_ = true;
				fadeTimer_ = 0;
			}

			if (isFadeIn_) {
				fadeTimer_++;
				fade_->SetColor({ 1.0f,1.0f,1.0f,1.0f - (float)fadeTimer_ / (float)kMaxFadeinTimer_ });
				if (fadeTimer_ >= kMaxFadeinTimer_) {
					isFadeIn_ = false;
					fadeTimer_ = 0;

					// 次のフロア
					currentFloor_++;
					Reset();
				}
			} else if (isFadeOut_) {
				fadeTimer_++;
				fade_->SetColor({ 1.0f,1.0f,1.0f,(float)fadeTimer_ / (float)kMaxFadeoutTimer_ });
				if (fadeTimer_ >= kMaxFadeoutTimer_) {
					isFadeOut_ = false;

					if (player_->IsDead() || currentFloor_ == 3) {
						Reset();

						if (isShowResult_) {
							scene.SceneChange("Game");
							return;
						} else {
							// ゲームオーバーまたはクリア
							isShowResult_ = true;

							fadeTimer_ = 0;
							isFadeIn_ = true;
						}
					} else {
						fadeTimer_ = 0;
						isFadeIn_ = true;
					}
				}

			}
		}
	}

	mapTile_->UpdateMapChange(!enemyManager_->GetEnemies().empty(), enableEditMode_);
	enemyManager_->SpawnCheck(player_->GetTransform().translate, mapCheck_.get());

	// カメラ行列更新
	camera_->Update(debugCamera_.get());
	debugCamera_->Update();

	if (enableEditMode_) {
		isLoaded_ = false;
	} else {
		// 編集の反映,再配置
		itemManager_->Load();
		if (!isLoaded_) {
			enemyManager_->Load(weaponManager_.get());
		}

		isLoaded_ = true;

	}
}

void GameScene::Draw() {
	BaseScene::Draw();

	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
	if (isShowResult_) {
		render.SetPostEffectType(PostEffectType::Grayscale);
	} else {
		render.SetPostEffectType(PostEffectType::Outline);
	}
	render.DrawSkybox(skybox_.get()); // パーティクルを後に描画したい

	player_->Draw(camera_.get());
	enemyManager_->Draw();
	bulletManager_->Draw(camera_.get());
	itemManager_->Draw(camera_.get());
	effectManager_->Draw(camera_.get());

	// ui
	if (!enableEditMode_) {
		uiDrawer_->Draw();
	}

	// 結果(プレイ画面の上から)
	if (isShowResult_) {
		render.DrawSprite(resultBG_.get());
		render.DrawSprite(resultCursor_.get());
	}

	if (!enableEditMode_) {
		fade_->SetSize(Vector2{1280, 720} + Vector2{ 20,80 });
		render.DrawSprite(fade_.get());
	}


#ifdef USE_IMGUI
	ImGui::Begin("Weapon");
	if (ImGui::Button("Pistol")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 0);
	};
	if (ImGui::Button("AssaultRifle")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 1);
	};
	if (ImGui::Button("Shotgun")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 2);
	};
	if (ImGui::Button("Flame")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 3);
	};
	if (ImGui::Button("Wave")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 4);
	};
	if (ImGui::Button("Orbit")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 5);
	};
	if (ImGui::Button("Charge")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 6);
	};
	if (ImGui::Button("Accel")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 7);
	};
	if (ImGui::Button("Sniper")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 8);
	};
	if (ImGui::Button("Burst")) {
		itemManager_->SpawnAndDrop(player_->GetTransform().translate, 9);
	};
	ImGui::End();

#endif
}

void GameScene::Reset() {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	enemyManager_->Reset();
	itemManager_->Reset();
	bulletManager_->Reset();
	player_->Stop();

	scene.SceneLoad("Resources/Debug/SceneEditor/SceneData.json");

	Vector3 pos;
	pos = { 5 * 1.5f,0,5 * 1.5f };
	pos.y = 0;

	// プレイヤー位置
	player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, pos });

	isLoaded_ = false;
	mapCheck_->Initialize(mapTile_->GetMap(), mapTile_->GetTileSize());
}
