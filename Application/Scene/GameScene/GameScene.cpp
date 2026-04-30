#include "GameScene.h"
#include <numbers>

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& asset = ctx.Asset();

	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize();
	camera_ = std::make_unique<Camera>();
	camera_->transform_.rotate = { 1.0f,0,0 };

	audio.SoundLoad(L"Resources/Sounds/SE/explosion.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/shoot.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/fire.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/floorClear.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/fall.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/warp.mp3");
	audio.SoundLoad(L"Resources/Sounds/SE/hit.mp3");

	playerModel_ = asset.LoadModel("Resources/Player", "player.obj");
	playerShadowModel_ = asset.LoadModel("Resources/Player", "player.obj");
	MaterialData data = playerModel_->GetMaterial(0)->GetData();
	data.color = { 1,1,1,0.7f };
	playerModel_->GetMaterial(0)->SetData(data);

	// マップ
	wall_ = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 500);
	wallShadow_ = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 500);
	floor_ = asset.LoadInstancedModel("Resources/Floor", "floor.obj", 500);
	goal_ = asset.LoadModel("Resources/Tiles", "sphere.obj");

	mapTile_ = std::make_unique<MapTile>();
	mapTile_->Initialize(std::move(wall_), std::move(floor_), std::move(goal_));

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
	collisionChecker_->Inititalize(effectManager_.get());

	// プレイヤー
	player_ = std::make_unique<Player>();
	player_->Initialize(std::move(playerModel_), std::move(playerShadowModel_));

	// 敵
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize();

	// 弾
	bulletManager_ = std::make_unique<BulletManager>();

	// UI描画システム
	uiDrawer_ = std::make_unique<UIDrawer>();
	uiDrawer_->Initialize(player_.get());

	// 天球
	skydome_ = std::make_unique<Model>();
	skydome_ = asset.LoadModel("Resources/Skydome", "skydome.obj", false);

	// 雲
	cloud_ = std::make_unique<Model>();
	cloud_ = asset.LoadModel("Resources/Cloud", "Cloud.obj", false);
	cloud_->SetTranslate({ 0,-20,0 });
	data = cloud_->GetMaterial(0)->GetData();
	data.color.w = 1.0f;
	cloud_->GetMaterial(0)->SetData(data);

	camera_->transform_.translate = player_->GetTransform().translate + Vector3{ 0,0,-cameraDistance_ };

	// フェード
	fade_ = std::make_unique<Sprite>();
	fade_ = asset.LoadSprite("resources/Debug/white1x1.png");
	fade_->SetSize(ctx.GetWindowSize() + Vector2{ 20,80 });
	fade_->SetColor({ 1.0f,1.0f,1.0f,0.0f });

	resultBG_ = std::make_unique<Sprite>();
	resultBG_ = asset.LoadSprite("resources/Result/result.png");
	resultBG_->SetSize(ctx.GetWindowSize() + Vector2{ 20,80 });
	resultBG_->SetColor({ 1, 1, 1, 0.7f });

	resultCursor_ = std::make_unique<Sprite>();
	resultCursor_ = asset.LoadSprite("resources/Result/cursor.png");
	resultCursor_->SetSize({ 48,56 });
	resultCursor_->SetColor({ 1, 1, 1, 0.7f });

	// マップ判定
	mapCheck_ = std::make_unique<MapCheck>();

	Reset();
}

void GameScene::Update() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	auto& audio = ctx.Audio();
	auto& scene = ctx.Scene();

	if (!isShowResult_) {
		if (isPause_) {
			// ポーズ中
			if (input.IsRelease(DIK_ESCAPE)) {
				isPause_ = false;
			}

		} else {
			MaterialData data = cloud_->GetMaterial(0)->GetData();
			data.uvTransform.m[3][1] += 0.001f;
			cloud_->GetMaterial(0)->SetData(data);

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
			camera_->Update(debugCamera_.get());
			debugCamera_->Update();

			// 敵
			enemyManager_->Update(mapCheck_.get(), player_.get(), bulletManager_.get(), camera_.get());

			// 弾の処理
			bulletManager_->Update(mapCheck_.get());
			for (const auto& bullet : bulletManager_->GetBullets()) {

				// 当たり判定
				collisionChecker_->Check(player_.get(), bullet, camera_.get());

				for (auto enemy : enemyManager_->GetEnemies()) {
					collisionChecker_->Check(enemy, bullet, camera_.get());
				}
			}

			// アイテム
			itemManager_->Update(player_.get());

			// マップ
			mapTile_->Update(enemyManager_->GetEnemies().size() == 0);

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
						scene.SceneChange("Title");
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
			// ポーズ(フェード中不可)
		/*	if (input.IsRelease(DIK_ESCAPE)) {
				isPause_ = true;
			}*/
		}

	} else {
		// リザルト
		if (resultArrowMove_ < 1.0f) {
			resultArrowMove_ += 1.0f / 60.0f;
		}

		resultTime_ += 1.0f / 60.0f;
		float endX = 0;
		switch (currentFloor_) {
		case 0:
			endX = 200;
			break;
		case 1:
			endX = 465;
			break;
		case 2:
			endX = 754;
			break;
		case 3:
			endX = 1016;
			break;
		case 4:
			endX = 1300;
			break;
		}
		float sinWave_ = sinf(10.0f * float(std::numbers::pi) * resultTime_);
		resultCursor_->SetPosition({ endX * resultArrowMove_,180 + sinWave_ * 10 });

		if (input.IsTrigger(DIK_SPACE)) {
			isFadeOut_ = true;
			fadeTimer_ = 0;
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
						scene.SceneChange("Title");
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

		}
	}
}

void GameScene::Draw() {
	auto& render = GameContext::GetInstance().Render();

	render.DrawModel(skydome_.get(), camera_.get());
	render.DrawModel(cloud_.get(), camera_.get(), BlendMode::Add);
	mapTile_->Draw(camera_.get());
	player_->Draw(camera_.get());
	enemyManager_->Draw(camera_.get());
	bulletManager_->Draw(camera_.get());
	itemManager_->Draw(camera_.get());
	effectManager_->Draw(camera_.get());

	if (currentFloor_ == 0) {
		// チュートリアル表示
	}

	// ui
	uiDrawer_->Draw();

	// 結果(プレイ画面の上から)
	if (isShowResult_) {
		render.DrawSprite(resultBG_.get());
		render.DrawSprite(resultCursor_.get());
	}

	render.DrawSprite(fade_.get());
}

void GameScene::Reset() {
	auto& ctx = GameContext::GetInstance();

	enemyManager_->Reset();
	itemManager_->Reset();
	bulletManager_->Reset();
	player_->Stop();

	int floorType = 0;
	std::string tilePath;
	std::string itemPath;
	std::string enemyPath;
	switch (currentFloor_) {
	case 0:
		// チュートリアルステージ
		floorType = 0;

		// マップを構築
		tilePath = "Resources/MapData/Floor" + std::to_string(floorType) + ".csv";
		mapTile_->LoadCSV(tilePath);

		// プレイヤー位置
		player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, {3,0,3} });

		// その階の敵とアイテム
		itemPath = "Resources/MapData/Item" + std::to_string(floorType) + ".csv";
		itemManager_->LoadCSV(itemPath, mapTile_->GetTileSize());
		enemyPath = "Resources/MapData/Enemy" + std::to_string(floorType) + ".csv";
		enemyManager_->LoadCSV(enemyPath, mapTile_->GetTileSize(), weaponManager_.get());

	case 1:
		floorType = ctx.RandomInt(1, 2);

		// マップを構築
		tilePath = "Resources/MapData/Floor" + std::to_string(floorType) + ".csv";
		mapTile_->LoadCSV(tilePath);

		// プレイヤー位置
		player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, {3,0,3} });

		// その階の敵とアイテム
		itemPath = "Resources/MapData/Item" + std::to_string(floorType) + ".csv";
		itemManager_->LoadCSV(itemPath, mapTile_->GetTileSize());
		enemyPath = "Resources/MapData/Enemy" + std::to_string(floorType) + ".csv";
		enemyManager_->LoadCSV(enemyPath, mapTile_->GetTileSize(), weaponManager_.get());

		break;

	case 2:
		floorType = ctx.RandomInt(3, 4);

		// マップを構築
		tilePath = "Resources/MapData/Floor" + std::to_string(floorType) + ".csv";
		mapTile_->LoadCSV(tilePath);

		// プレイヤー位置
		player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, {3,0,3} });

		// その階の敵とアイテム
		itemPath = "Resources/MapData/Item" + std::to_string(floorType) + ".csv";
		itemManager_->LoadCSV(itemPath, mapTile_->GetTileSize());
		enemyPath = "Resources/MapData/Enemy" + std::to_string(floorType) + ".csv";
		enemyManager_->LoadCSV(enemyPath, mapTile_->GetTileSize(), weaponManager_.get());

		break;

	case 3:
		floorType = ctx.RandomInt(10, 11);

		// マップを構築
		tilePath = "Resources/MapData/Floor" + std::to_string(floorType) + ".csv";
		mapTile_->LoadCSV(tilePath);

		// プレイヤー位置
		player_->SetTransform({ { 1,1,1 }, { 0,0,0 }, {3,0,3} });

		// その階の敵
		enemyPath = "Resources/MapData/Enemy" + std::to_string(floorType) + ".csv";
		enemyManager_->LoadCSV(enemyPath, mapTile_->GetTileSize(), weaponManager_.get());

		break;
	}

	mapCheck_->Initialize(mapTile_->GetMap(), mapTile_->GetTileSize());
}
