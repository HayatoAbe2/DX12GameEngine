#pragma once

struct EnemyStatus {
	// hp
	float hp = 10;
	// 半径
	float radius = 0.5f;
	// 移動の速さ
	float moveSpeed = 0.1f;
	// 検知範囲
	float defaultSearchRadius = 8.0f;
	// 攻撃開始範囲
	float attackRadius = 9.0f;
	// スタン時間を減らす量
	int stunResist = 0;
	// 飛行する
	bool canFly = false;
};