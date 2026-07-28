#pragma once
#include "GameCommon.h"
#include "Rarity.h"
#include "Character/Player/Player.h"

// 落ちているアイテム
class WorldItem {
public:
	WorldItem(Vector3 pos, Rarity rarity, bool isForSale = false);
	virtual ~WorldItem();

	virtual void Update() = 0;
	virtual void Draw();

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

	// 売り物フラグ
	bool isForSale_ = false;
	// 売値
	int price_ = 10;
	// スプライト
	std::unique_ptr<Sprite> coinIcon_;
	std::array<std::unique_ptr<Sprite>, 3> numbers_;
	float numSpacing_ = 27.0f;
	float digitSize_ = 32.0f;
	float iconSize_ = 32.0f;
	// 画像上の数字サイズ
	float digitWidth_ = 64.0f;

	// オフセット
	float valueOffsetY_ = -1.0f;

	float valueRangeMin_ = 0.85f;
	float valueRangeMax_ = 1.15f;
};

