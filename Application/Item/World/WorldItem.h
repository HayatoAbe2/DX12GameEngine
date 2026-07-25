#pragma once
#include "GameCommon.h"
#include "Rarity.h"
#include "Character/Player/Player.h"

// 落ちているアイテム
class WorldItem {
public:
	WorldItem(Vector3 pos, Rarity rarity);
	~WorldItem();

	virtual void Update() = 0;
	virtual void Draw() = 0;

	// 取得されたとき
	virtual void OnPickup(Player* player) = 0;

	Vector2 GetPosition() { return position_; }
	bool CanInteract() { return canInteract_; }
	bool CanAutoGet() { return canAutoGet_; }

	bool IsDead() { return isDead_; }
	void Erase();

protected:
	Vector2 position_{};

	// ライトのインデックス
	int lightIndex_ = 0;

	bool isDead_ = false;

	// レア度
	Rarity rarity_{};

	// インタラクト
	bool canInteract_ = true;

	// 触れたら自動取得
	bool canAutoGet_ = false;
};

