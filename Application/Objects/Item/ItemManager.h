#pragma once
#include "GameCommon.h"
#include "Objects/Item/World/WorldItem.h"
#include "Objects/Item/World/Chest/Chest.h"
#include "Rarity.h"

class Player;
class WeaponManager;
class EnemyManager;
class BulletManager;
class Camera;

class ItemManager {
public:
	void Initialize(WeaponManager* weaponManager, EnemyManager* enemyManager, BulletManager* bulletManager);
	void Update(Player* player, bool isCombat);
	void Draw();

	// 近くのアイテムにインタラクト
	void Interact(Player* player);

	// 新しく出現させる
	void SpawnWeapon(Vector3 pos,int index,Rarity rarity, bool isForSale);
	void SpawnMoney(Vector3 pos,int amount);
	void SpawnPassive(Vector3 pos, bool isForSale);
	void SpawnChest(Vector3 pos);

	// 持っていたものを落とす
	void Drop(Vector3 pos,std::unique_ptr<Weapon> weapon);

	// 配置リセット
	void Reset();

	// 次フレームに配置(描画段階での生成をしないため)
	void SetSpawn(Vector3 pos, int index);

	// シーンを見て配置
	void Load();
	
private:
	// 宝箱からの出現
	void ChestItemDrop(Chest* chest);

	// 戦闘中
	bool isCombat_ = false;

	std::unique_ptr<Sprite> controlKey_ = nullptr;
	std::unique_ptr<Sprite> controlPad_ = nullptr;
	WeaponManager* weaponManager_ = nullptr;
	EnemyManager* enemyManager_ = nullptr;
	BulletManager* bulletManager_ = nullptr;

	// 落ちているアイテム
	std::vector<std::unique_ptr<WorldItem>> items_;
	std::vector<std::unique_ptr<Chest>> chests_;

	// インタラクト可能
	bool canInteract_ = true;
	const float kInteractRange = 2.0f; // インタラクト範囲

	std::vector<bool> weaponSpawned_;
	std::vector<bool> weaponSaleSpawned_;
	std::vector<bool> passiveSpawned_;
	std::vector<bool> passiveSaleSpawned_;
	int nextSpawnIndex_ = -1;
	Vector3 nextSpawnPos_{};

	std::vector<int> usedPassiveNumbers_;
	std::vector<int> pickedPassiveNumbers_;
};

