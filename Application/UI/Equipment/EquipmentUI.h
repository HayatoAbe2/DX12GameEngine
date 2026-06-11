#pragma once
#include "GameCommon.h"

class Player;
class EquipmentUI {
public:
	void Initialize();
	void Update(Player* player);
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

	// 操作
	std::unique_ptr<Sprite> control_ = nullptr;
	SpriteData controlUIData_ = { {640 - 24 + 100,710 - 65}, { 48,65 } };

};

