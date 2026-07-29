#include "Counter.h"

Counter::Counter(std::unique_ptr<Sprite> sprite) : Passive(std::move(sprite)) {
}

void Counter::OnHit(const Vector2& pos, BulletManager* bulletManager, Character* from) {
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
	data.color = {0.5f,0.5f,0.5f,0.8f};

	int count = 8;
    for (int i = 0; i < count; i++) {
        float angle = (2.0f * float(std::numbers::pi) / count) * i;
        Vector2 shotDir = {cosf(angle), sinf(angle)};

		// 弾生成
        std::unique_ptr<Bullet> newBullet = std::make_unique<Bullet>(pos, shotDir, data, from);
        bulletManager->AddBullet(std::move(newBullet));
    }
}
