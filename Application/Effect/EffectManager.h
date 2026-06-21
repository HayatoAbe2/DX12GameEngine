#pragma once
#include "GameCommon.h"
#include "Effect/Hit/HitEffect.h"
#include "Effect/Explode/ExplodeEffect.h"

class Camera;

// 敵や弾が消えたあとで続くエフェクト/パーティクルの管理
class EffectManager {
public:
	void Initialize();
	void Update();
	void Draw(Camera*camera);

	void SpawnHitEffect(const Vector3& pos);
	void SpawnExplodeEffect(const Vector3& pos);
private:
	std::vector<std::unique_ptr<Model>> hitEffect_;
	std::vector<std::unique_ptr<HitEffect>> hitSpark_;
	std::vector<std::unique_ptr<ExplodeEffect>> explode_;
	float hitEffectUpScaleSpeed_ = 0.2f;

	
};

