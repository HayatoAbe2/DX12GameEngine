#pragma once
#include "GameCommon.h"
#include "Objects/Weapon/WeaponStatus.h"

class Weapon;

class WeaponManager {
public:
	void Initialize();

	std::unique_ptr<Weapon> GetWeapon(int index, Rarity rarity = Rarity::Common);
	std::unique_ptr<Weapon> GetEnemyAttack(int index);
	std::unique_ptr<Weapon> GetEnemyAttack(const std::string& name);

	enum class WEAPON {
		Pistol,
		AssaultRifle,
		Shotgun,
		Fireball,
		Wavegun,
		OrbitStaff,
		ChargeGun,
		AccelGun,
		Sniper,
		Burstgun,

		Count
	};

	enum class ENEMYATTACK {
		Fireball,
		AssaultRifle,
		Pistol,
		ThreeWay,
		FourWay,
		LargeFireball,

		Count
	};


private:

	// json読み込み
	void LoadJson(const std::string& path, bool isEnemyAttack = false);
	// 読み込んだデータ
	std::unordered_map<std::string, WeaponData> weaponDataMap;
	std::unordered_map<std::string, WeaponData> enemyAttackDataMap;

	const std::string WeaponNames[(int)WEAPON::Count]{
		"Pistol",
		"AssaultRifle",
		"Shotgun",
		"Spellbook",
		"Wavegun",
		"OrbitStaff",
		"ChargeGun",
		"AccelGun",
		"Sniper",
		"Burstgun"
	};

	const std::string EnemyAttackNames[(int)ENEMYATTACK::Count]{
		"Fireball",
		"AssaultRifle",
		"Pistol",
		"3way",
		"4way",
		"LargeFireball"
	};
};

