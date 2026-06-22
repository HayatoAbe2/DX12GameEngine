#pragma once
#include "GameCommon.h"
#include "WeaponStatus.h"

class Weapon;

class WeaponManager {
public:
	void Initialize();

	std::unique_ptr<Weapon> GetWeapon(int index,Rarity rarity = Rarity::Common);
	
	enum class WEAPON {
		Pistol,
		AssaultRifle,
		Shotgun,
		FireBall,
		Wavegun,
		OrbitStaff,
		ChargeGun,
		AccelGun,
		Sniper,
		Burstgun,
	};

private:
	// json読み込み
	void LoadJson(const std::string& path);
	// 読み込んだデータ
	std::unordered_map<std::string, WeaponData> weaponDataMap;
};

