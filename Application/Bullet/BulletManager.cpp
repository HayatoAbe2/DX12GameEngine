#include "BulletManager.h"
#include "Map/MapCheck.h"
#include "Bullet.h"

void BulletManager::Update(MapCheck* mapCheck) {
	for (const auto& bullet : bullets_) {
			bullet->Update(mapCheck);
	}
	bullets_.erase(
		std::remove_if(bullets_.begin(), bullets_.end(),
			[](const std::unique_ptr<Bullet>& bullet) {
				return bullet->CanErase();
			}
		),
		bullets_.end()
	);
}

void BulletManager::Draw(Camera* camera) {
	for (const auto& bullet : bullets_) {
		bullet->Draw(camera);
	}
}

void BulletManager::Reset() {
	bullets_.clear();
}

void BulletManager::AddBullet(std::unique_ptr<Bullet> newBullet) {
	bullets_.push_back(std::move(newBullet));
}

std::vector<Bullet*> BulletManager::GetBullets() {
	std::vector<Bullet*> bullets;
	for (const auto& bullet : bullets_) {
		bullets.push_back(bullet.get());
	}
	return bullets;
}
