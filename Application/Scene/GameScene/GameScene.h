#pragma once
#include "Engine/Scene/BaseScene/BaseScene.h"

#include "Result/Result.h"
#include "Character/Player/Player.h"
#include "Character/Enemy/EnemyManager/EnemyManager.h"
#include "Bullet/BulletManager.h"
#include "Effect/EffectManager.h"
#include "Map/MapTile.h"
#include "Map/MapCheck.h"
#include "Weapon/WeaponManager/WeaponManager.h"
#include "Item/ItemManager.h"
#include "CollisionSystem/CollisionSystem.h"
#include "UI/UIDrawer/UIDrawer.h"
#include "Map/FloorManager/FloorManager.h"
#include "Fade/Fade.h"
#include "GameCameraController/GameCameracontroller.h"
#include "RandomSettings/RandomSettings.h"

#include "Editor/SpawnRateEditor/SpawnRateEditor.h"

// ゲームシーン
class GameScene : public BaseScene {
public:

	~GameScene() override;

	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// リセット
	void Reset();

	// 次の部屋に移動
	void MoveToNextRoom(Direction direction);

private:
	void LoadSound();

	enum class Phase {
		GAME,
		RESULT
	};
	Phase phase_ = Phase::GAME;
	std::unique_ptr<Result> result_;

	std::shared_ptr<Texture> skybox_ = nullptr;

	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_ = nullptr;

	// 弾
	std::unique_ptr<BulletManager> bulletManager_ = nullptr;

	// エフェクト
	std::unique_ptr<EffectManager> effectManager_ = nullptr;

	// マップ
	std::unique_ptr<MapTile> mapTile_ = nullptr;

	// マップ判定
	std::unique_ptr<MapCheck> mapCheck_ = nullptr;

	// 武器マネージャー
	std::unique_ptr<WeaponManager> weaponManager_ = nullptr;

	// アイテムマネージャー
	std::unique_ptr<ItemManager> itemManager_ = nullptr;

	// 当たり判定
	std::unique_ptr<CollisionSystem> collisionSystem_ = nullptr;

	// 確率設定
	std::unique_ptr<RandomSettings> randomSettings_ = nullptr;

	// UI
	std::unique_ptr<UIDrawer> uiDrawer_ = nullptr;

	// カメラ
	std::unique_ptr<Camera> camera_ = nullptr;

	// デバッグカメラ
	std::unique_ptr <DebugCamera> debugCamera_ = nullptr;

	// ポーズ
	bool isPause_ = false;

	bool isLoaded_ = false;

	// フロア
	std::unique_ptr<FloorManager> floorManager_;
	bool isRoomMoving_ = false;
	Direction nextDirection_;

	// フェード
	std::unique_ptr<Fade> fade_ = nullptr;

	// カメラ制御
	std::unique_ptr<GameCameraController> cameraController_ = nullptr;

	// 敵出現確率
	std::unique_ptr<SpawnRateEditor> spawnEditor_ = nullptr;
};
