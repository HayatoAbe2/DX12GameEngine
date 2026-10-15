#include "ExtraAttack.h"

ExtraAttack::ExtraAttack(std::unique_ptr<Sprite> sprite, std::unique_ptr<Sprite> explain, PassiveContext ctx) : Passive(std::move(sprite), std::move(explain), passiveCtx_) {
}

void ExtraAttack::OnEliminate(const Vector2& enemyPos, Character* self) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	// 弾データ
	BulletData data;
	data.damage = 6.0f;
	data.radius = 0.5f;
	data.speed = 30;
	data.lifeTime = 60;
	data.knockback = 0.3f;
	data.color = { 1.0f,0.0f,1.0f,0.8f };

	int count = 8;
	for (int i = 0; i < count; i++) {
		float angle = (2.0f * float(std::numbers::pi) / count) * i;
		Vector2 shotDir = { cosf(angle), sinf(angle) };

		// 弾生成
		std::unique_ptr<Bullet> newBullet = std::make_unique<Bullet>(enemyPos, shotDir, data, self);
		passiveCtx_.bulletManager->AddBullet(std::move(newBullet));
	}
}
