#pragma once
#include "GameCommon.h"
#include "Objects/Weapon/WeaponStatus.h"

class Weapon;

/// <summary>
/// 武器生成
/// </summary>
class WeaponManager {
public:
	void Initialize();

	// 武器データを読み込み直す(Resources/Data/Weapons内のjsonをすべて読む)
	void Reload();

	std::unique_ptr<Weapon> GetWeapon(int index, Rarity rarity = Rarity::Common);
	std::unique_ptr<Weapon> GetWeaponByName(const std::string& name, Rarity rarity = Rarity::Common);
	std::unique_ptr<Weapon> GetEnemyAttack(int index);
	std::unique_ptr<Weapon> GetEnemyAttack(const std::string& name);

	// 読み込まれている武器名(ソート済み)
	std::vector<std::string> GetWeaponNames() const;
	// 敵用の武器名(末尾の _Enemy を除いたもの。GetEnemyAttack に渡す名前)
	std::vector<std::string> GetEnemyAttackNames() const;
	// 敵用武器のデータ(無ければ nullptr)
	const WeaponData* GetEnemyAttackData(const std::string& name) const;

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
	void LoadJson(const std::string& path);
	// 武器の生成(モデルの読み込み)
	std::unique_ptr<Weapon> CreateWeapon(const WeaponData& data);
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
		"ThreeWay",
		"FourWay",
		"LargeFireball"
	};
};

