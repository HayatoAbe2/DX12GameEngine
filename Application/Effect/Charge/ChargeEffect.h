#pragma once
#include "GameCommon.h"

/// <summary>
/// チャージ中の弾の表示
/// </summary>
class ChargeEffect {
public:
	ChargeEffect(Vector4 color);

	void Update(Vector2 center, float radius);
	void Draw();

private:
	// パーティクル
	std::unique_ptr<ParticleSystem> particle_;
	const int kParticleCount_ = 150;
	float particleRange_ = 0.7f;
};

