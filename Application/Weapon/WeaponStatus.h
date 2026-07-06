#pragma once
#include "Rarity.h"
#include "Weapon/Trait/BulletTrait.h"
#include "GameCommon.h"
#include <variant>

struct NormalParam {};
struct ShotgunParam {
	// 発射数
	int8_t pelletCount;

	// 拡散角度
	float maxAngle = 0;
};

struct WeaponStats {
	// ダメージ
	float damage;
	// 射撃中減速率
	float weight;
	// 弾の大きさ
	float bulletSize;
	// 弾速
	float bulletSpeed;
	// 射撃クールダウン
	float shootCoolTime;
	// 弾の生存時間
	int bulletLifeTime;
	// ノックバック
	float knockback;
	// 最大チャージ(弾数)
	float maxCharge;
	// 1発分のチャージ時間
	float chargeTime;
};

struct WeaponData {
	std::string name;
	std::string modelName;

	// 共通の基本ステータス
	WeaponStats stats;
	// 種類ごとの特殊パラメータ
	std::variant<NormalParam, ShotgunParam> weaponSpecialData;
	// 属性(弾の性質)
	BulletTraits bulletTraits;
	// 弾の色
	Vector4 bulletColor;
	// レア度(表示に影響)
	Rarity rarity;
};

// 強化
struct WeaponModifier {
	WeaponStats add;
	WeaponStats multiplier{
		1,1,1,1,1,1,1
	};
};
