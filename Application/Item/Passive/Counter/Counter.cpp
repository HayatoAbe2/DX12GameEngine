#include "Counter.h"
#include <Bullet/RapidBullet.h>

Counter::Counter(Sprite* sprite) : Passive(sprite) {
}

void Counter::OnHit(const Vector2& pos, BulletManager* bulletManager, Character* from) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& audio = ctx.Audio();

	WeaponData wd;
	wd.stats.damage = 6.0f;
	wd.stats.bulletSize = 0.5f;
	wd.stats.bulletSpeed = 30;
	wd.stats.bulletLifeTime = 60;
	wd.stats.knockback = 0.3f;
	wd.bulletColor = {0.5f,0.5f,0.5f,0.8f};

    for (int i = 0; i < 8; i++) {
        float angle = (2.0f * float(std::numbers::pi) / 8) * i;

        Vector3 shotDir = {
            cosf(angle),
            0,
            sinf(angle)
        };

        auto bullet = asset.LoadModel("Resources/Bullets", "gunBullet.obj");

        bullet->SetTranslate({ pos.x, 0.5f, pos.y });

        std::unique_ptr<RapidBullet> newBullet =
            std::make_unique<RapidBullet>(
                std::move(bullet),
                shotDir,
                wd,
                from
            );

        newBullet->Initialize();
        bulletManager->AddBullet(std::move(newBullet));
    }
}
