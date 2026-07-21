 #include "Lightning.h"

Lightning::Lightning(Sprite* sprite) : Passive(sprite) {
}

void Lightning::OnDealDamage(const Vector2& pos, EnemyManager* enemyManager) {
	Circle c1;
	c1.center = pos;
	c1.radius = 10.0f;
	for (auto& e : enemyManager->GetEnemies()) {
		Circle c2;
		c2.center = ToXZ(e->GetTransform().translate);
		c2.radius = e->GetRadius();

		if (CheckCollision(c1, c2)) {
			e->Hit(1.0f, pos, 0.1f);
		}
	}
}