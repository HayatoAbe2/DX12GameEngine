#pragma once
#include "GameCommon.h"
#include "Item/World/WorldWeapon/WorldWeapon.h"
#include "Item/World/WorldPassive/WorldPassive.h"
#include "Item/World/WorldMoney/WorldMoney.h"
#include "Rarity.h"

class Player;
class WeaponManager;
class Camera;

class ItemManager {
public:
	void Initialize(WeaponManager* weaponManager);
	void Update(Player* player);
	void Draw();

	// 近くのアイテムにインタラクト
	void Interact(Player* player);

	// 新しく出現させる
	void SpawnWeapon(Vector3 pos,int index,Rarity rarity);
	void SpawnMoney(Vector3 pos,int amount);

	// 持っていたものを落とす
	void Drop(Vector3 pos,std::unique_ptr<Weapon> weapon);

	// 配置リセット
	void Reset();

	// 次フレームに配置(描画段階での生成をしないため)
	void SetSpawn(Vector3 pos, int index);

	// シーンを見て配置
	void Load();
	
private:
	std::unique_ptr<Sprite> controlKey_ = nullptr;
	std::unique_ptr<Sprite> controlPad_ = nullptr;
	WeaponManager* weaponManager_ = nullptr;

	// 落ちているアイテム
	std::vector<std::unique_ptr<WorldItem>> items_;

	bool canInteract_ = true;

	std::vector<bool> spawned_;
	int nextSpawnIndex_ = -1;
	Vector3 nextSpawnPos_{};
};

