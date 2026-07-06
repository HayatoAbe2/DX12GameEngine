#pragma once
#include "GameCommon.h"

class Player;
class EquipmentUI {
public:
	void Initialize(Player* player);
	void Update();
	void Draw();

private:
	struct SpriteData {
		Vector2 pos;
		Vector2 size;
	};

	Player* player_ = nullptr;

	Sprite* equipment_ = nullptr;
	Sprite* equipment2_ = nullptr;

	std::unique_ptr<Sprite> equipAssaultRifle_ = nullptr;
	std::unique_ptr<Sprite> equipPistol_ = nullptr;
	std::unique_ptr<Sprite> equipShotgun_ = nullptr;
	std::unique_ptr<Sprite> equipFireBall_ = nullptr;
	std::unique_ptr<Sprite> equipWavegun_ = nullptr;

	std::unique_ptr<Sprite> equipAssaultRifle2_ = nullptr;
	std::unique_ptr<Sprite> equipPistol2_ = nullptr;
	std::unique_ptr<Sprite> equipShotgun2_ = nullptr;
	std::unique_ptr<Sprite> equipFireBall2_ = nullptr;
	std::unique_ptr<Sprite> equipWavegun2_ = nullptr;

	std::unique_ptr<Sprite> ammo_ = nullptr;
	std::unique_ptr<Sprite> ammo2_ = nullptr;

	// 操作
	std::unique_ptr<Sprite> shootUIKey = nullptr;
	std::unique_ptr<Sprite> shootUIPad = nullptr;
	std::unique_ptr<Sprite> swapUIKey = nullptr;
	std::unique_ptr<Sprite> swapUIPad = nullptr;
	SpriteData shootUIData_ = { {1030, 620}, { 64,64 } };
	SpriteData swapUIData_ = { {1110, 425}, { 50,50 } };

};

