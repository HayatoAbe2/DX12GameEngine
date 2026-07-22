#pragma once

// 貫通
struct Piercing {
	// 回数
	int count;
	int current;
};

// 跳弾
struct Ricochet {
	// 回数
	int count;
	int current;

	// 敵ヒット時反射
	bool onHitEnemy;

	// 敵に向かって反射
	bool toEnemy;
};

// 爆発
struct Explode {
	// 半径
	float radius;
	float t;
};

// 軌道回転
struct Orbit {
	// 速度
	float speed;

	// 現在角度
	float angle = 0;

	// 半径
	float radius;
};

// 蛇行
struct Wave {
	// 振幅
	float amplitude;

	// 速度
	float speed;

	// 最大角度
	float maxAngle;

	// 現在時間
	float time = 0;
};

// 加速
struct Accel {
	// 加速量 /秒
	float rate;
};

// ヒット時減速
struct Slow {
	// 減速量(%)
	float rate;

	// 時間
	float time;
};

// 炎ダメージ
struct Flame {
	// 与えるスタック数
	float stack;

	// 間隔
	float interval;

	// ダメージ
	float damage;
};

// 凍結
struct Freeze {
	// 与えるスタック数
	float stack;
};

// 電撃(周辺の敵にダメージ)
struct Volt {
	// 範囲(半径)
	float range;

	// 元と比較したダメージ割合
	float damage;
};

// 移動の挙動
struct Move {
	std::optional<Orbit> orbit;
	std::optional<Wave> wave;
	std::optional<Accel> accel;
};

// 何かにヒット時
struct OnHitAnything {
	std::optional<Explode> explode;
};

// 壁ヒット時
struct OnHitWall {
	std::optional<Ricochet> ricochet;
};

// 敵ヒット時
struct OnHitEnemy {
	std::optional<Piercing> piercing;
	std::optional<Slow> slow;
	std::optional<Flame> flame;
	std::optional<Freeze> freeze;
	std::optional<Volt> volt;
};

// 弾の性質(発動条件ごと)
struct BulletTraits {
	Move move;
	OnHitAnything onHitAnything;
	OnHitWall onHitWall;
	OnHitEnemy onHitEnemy;
};