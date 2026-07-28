#pragma once
#include "Rarity.h"
#include "Weapon/Trait/BulletTrait.h"
#include "GameCommon.h"
#include <Timer/Timer.h>

// 複数弾
struct MultiShotParam {
	// 発射数
	int8_t count;

	// 拡散角度(度)
	float maxAngle;

	// 範囲内でランダムに飛ぶ
	bool random;
};
// 溜め
struct ChargeParam {
	// 最大までの時間
	float time;
	float currentTime = 0;

	// ダメージ上昇率
	float damage;

	bool charging = false;
};
// バースト
struct BurstParam {
	// 連続射撃回数
	int count;
	int currentCount = 0;

	// 間隔
	float interval;
	Timer timer;

	// 方向
	Vector2 dir;
};

struct WeaponStats {
	// 弾のブレ

	// 減速率
	float weight;
	// 射撃クールダウン
	float shootCoolTime;
	// 最大チャージ(弾数)
	float maxCharge;
	// 1発分のチャージ時間
	float chargeTime;
};

// 弾のデータ
struct BulletData {
	// ダメージ
	float damage;
	// 弾の大きさ
	float radius;
	// 弾速
	float speed;
	// 弾の生存時間
	int lifeTime;
	// 与えるノックバック
	float knockback;

	// 属性(弾の性質)
	BulletTraits traits;
	// 弾の色
	Vector4 color;
};

// 特殊パラメータ
struct WeaponTrait {
	std::optional<MultiShotParam> multiShot;
	std::optional<ChargeParam> charge;
	std::optional<BurstParam> burst;
};

// 強化などの変更
enum class ModifierStats {
	damage,
	size,
	knockback,
	coolTime,

	count
};

// 計算方法
struct ModifierType {
	float add = 0;
	float multiply = 1;
};

struct WeaponData {
	std::string name;
	std::string modelName;

	// 共通の基本ステータス
	WeaponStats stats;

	// 種類ごとの特殊パラメータ
	WeaponTrait traits;

	// ステータス変化
	std::array<ModifierType, size_t(ModifierStats::count)> modifiers;

	// 弾のデータ
	BulletData bullet;

	// レア度
	Rarity rarity;
};