#pragma once
#include "GameCommon.h"
#include "Item.h"
#include "Rarity.h"

class Player;
class WeaponManager;
class Camera;

class ItemManager {
public:
	void Initialize(WeaponManager* weaponManager);
	void Update(Player* player);
	void Draw(Camera* camera);
	void Interact(Player* player);
	void Spawn(Vector3 pos,int index);
	void Spawn(Vector3 pos,int index,Rarity rarity);
	void Drop(Vector3 pos,std::unique_ptr<Weapon> weapon);
	void Reset();
	void LoadCSV(const std::string& filePath,const float tileSize);
	void Load();
	
private:
	std::unique_ptr<Sprite> controlKey_ = nullptr;
	std::unique_ptr<Sprite> controlPad_ = nullptr;
	WeaponManager* weaponManager_ = nullptr;
	std::vector<std::unique_ptr<Item>> items_;
	bool canInteract_ = true;

	bool spawned_ = false;
};

