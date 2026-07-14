#pragma once
#include "GameCommon.h"
#include "UI/Equipment/EquipmentUI.h"

class Player;
class UIDrawer {
public:
	// 初期化
	void Initialize(Player* player);
	// 更新
	void Update();
	// 描画
	void Draw();

	// プレイヤーUI(所持アイテムなど)
	void UpdatePlayerUI();
	// ステージUI(進捗)
	void UpdateStageUI();

private:
	Player* player_ = nullptr;

	struct SpriteData {
		Vector2 pos;
		Vector2 size;
	};

	// hp
	std::unique_ptr<Sprite> life_ = nullptr;
	std::unique_ptr<Sprite> damage_ = nullptr; // 減った分
	std::unique_ptr<Sprite> lifeBack_ = nullptr;
	float whiteLife_ = 0;
	float whiteGaugeSpeed_ = 0.5f;

	std::unique_ptr<EquipmentUI> equipment_;
	// 弾数(〇とかで表示,8個*2色目とか)
	std::unique_ptr<Sprite> ammo_ = nullptr;
	std::unique_ptr<Sprite> magazine_ = nullptr;
	SpriteData ammoUIData_ = { {640,380}, {100,22} }; 

	std::unique_ptr<Sprite> dodgeControlKey_ = nullptr;
	std::unique_ptr<Sprite> dodgeControlPad_ = nullptr;
	SpriteData dodgeControlUIData_ = { {1110,360}, { 50, 50 } };
	std::unique_ptr<Sprite> dodge_ = nullptr;
	SpriteData dodgeUIData_ = { {1171,358}, { 80, 53.3f } };

	// 階数
	std::unique_ptr<Sprite> currentFloor_ = nullptr;
	SpriteData floorUIData_ = { {640,320}, { 212,32 } };
};

