#include "Enemy.h"
#include "Map/MapCheck.h"
#include "EnemyStatus.h"
#include "Character/Player/Player.h"

#include "Character/Enemy/EnemyState/EnemyApproach/EnemyApproach.h"
#include "Character/Enemy/EnemyState/EnemyAttack/EnemyAttack.h"
#include "Character/Enemy/EnemyState/EnemyIdle/EnemyIdle.h"

#include <numbers>
#include <cmath>

Enemy::Enemy(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, Vector3 pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons) {
	model_ = std::move(model);
	model_->SetTranslate(pos);
	status_ = status;
	shadowModel_ = std::move(shadowModel);
	auto matData = shadowModel_->GetMaterial(0)->GetData();
	matData.color = { 0,0,0,1 };
	shadowModel_->GetMaterial(0)->SetData(matData);

	weapons_ = std::move(rWeapons);
	if (!weapons_.empty()) {
		currentWeapon_ = weapons_[0].get();
	}

	stunTimer_ = std::make_unique<Timer>();
	attackCoolTimer_ = std::make_unique<Timer>();
	slowTimer_ = std::make_unique<Timer>();
	hitColorTimer_ = std::make_unique<Timer>();

	// 初期State
	currentState_ = std::make_unique<EnemyIdle>();
}

void Enemy::Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager, Camera* camera) {
	auto& ctx = GameContext::GetInstance();

	if (hitColorTimer_->IsActive()) {
		hitColorTimer_->Update();
		if (hitColorTimer_->IsFinished()) {
			for (auto& mesh : model_->GetData()->meshes) {
				auto data = model_->GetMaterial(0)->GetData();
				data.color = { 1.0f,1.0f,1.0f,1.0f };
				model_->GetMaterial(0)->SetData(data);
				model_->GetMaterial(1)->SetData(data);
			}
		}
	}
	slowTimer_->Update();

	// 落下
	if (isFall_) {
		Fall();
		return;
	}

	if (stunTimer_->IsActive()) {
		Stun(mapCheck);
		return;
	}

	// 現在Stateの行動
	currentState_->Update(this, player, mapCheck, bulletManager);
	velocity_ = currentState_->GetVelocity() * ctx.GetDeltatime();

	if (slowTimer_->IsActive()) {
		velocity_ *= 0.4f;
	}

	// 速度をもとに移動
	Vector2 pos = { model_->GetTransform().translate.x,model_->GetTransform().translate.z };
	pos.x += velocity_.x;
	mapCheck->ResolveCollisionX(pos, status_.radius, status_.canFly);
	pos.y += velocity_.z;
	mapCheck->ResolveCollisionY(pos, status_.radius, status_.canFly);
	model_->SetTranslate({ pos.x,model_->GetTransform().translate.y,pos.y });
}

void Enemy::Stun(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	stunTimer_->Update();

	// ノックバック
	model_->SetScale({ 1,1,1 });
	float length = Length(knockbackVelocity_);
	length -= 0.05f;
	if (length < 0) { length = 0; }
	knockbackVelocity_ = Normalize(knockbackVelocity_) * length;

	// 速度をもとに移動
	Vector2 pos = { model_->GetTransform().translate.x,model_->GetTransform().translate.z };
	for (int i = 0; i < 3; ++i) { // 3回に分ける
		pos.x += knockbackVelocity_.x / 3.0f;
		mapCheck->ResolveCollisionX(pos, status_.radius, true);
		pos.y += knockbackVelocity_.z / 3.0f;
		mapCheck->ResolveCollisionY(pos, status_.radius, true);
	}

	if (!stunTimer_->IsActive() && !status_.canFly) {
		isFall_ = mapCheck->IsFall(pos);
		if (isFall_) { audio.SoundPlay(L"Resources/Sounds/SE/fall.mp3", false); }
	}
	model_->SetTranslate({ pos.x,model_->GetTransform().translate.y,pos.y });
}

void Enemy::Fall() {
	// 落下
	model_->SetTranslate(model_->GetTransform().translate - Vector3{ 0,0.7f,0 });
	if (model_->GetTransform().translate.y < -10.0f) {
		isDead_ = true;
	}
}

void Enemy::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// 影描画
	if (!isFall_) {
		Transform shadowTransform = model_->GetTransform();
		shadowTransform.scale.y = 0.0f;
		shadowTransform.translate.y = 0.01f;
		shadowModel_->SetTransform(shadowTransform);
		render.DrawModel(shadowModel_.get());
	}
	render.DrawModel(model_.get());
}

void Enemy::Hit(float damage, Vector3 from, const float knockback) {

	status_.hp -= damage;
	if (status_.hp <= 0) { isDead_ = true; }

	if (status_.stunResist < 10) {
		// 行動不能
		stunTimer_->Start(float(10 - status_.stunResist) / 60);

		// ノックバック
		knockbackVelocity_ = Normalize(model_->GetTransform().translate - from) * knockback;
	}

	// 待機状態なら発見させる
	if (dynamic_cast<EnemyIdle*>(currentState_.get())) {
		currentState_ = std::make_unique<EnemyApproach>();
	}

	// ダメージを受けたら赤くする
	auto data = model_->GetMaterial(0)->GetData();
	data.color = { 1.0f,0.2f,0.2f,1.0f };
	model_->GetMaterial(0)->SetData(data);
	model_->GetMaterial(1)->SetData(data);
	hitColorTimer_->Start(0.2f);
}