#include "Enemy.h"
#include "Map/MapCheck.h"
#include "EnemyStatus.h"
#include "Objects/Character/Player/Player.h"

#include "Objects/Character/Enemy/EnemyActionState/EnemyApproach/EnemyApproach.h"
#include "Objects/Character/Enemy/EnemyActionState/EnemyAttack/EnemyAttack.h"
#include "Objects/Character/Enemy/EnemyActionState/EnemyIdle/EnemyIdle.h"

#include <numbers>
#include <cmath>

Enemy::Enemy(std::unique_ptr<Model> model, std::unique_ptr<Model> shadowModel, Vector3 pos, EnemyStatus status, std::vector<std::unique_ptr<Weapon>> rWeapons) {
	status_ = status;
	maxHp_ = status_.hp;
	hp_ = status_.hp;
	radius_ = status_.radius;
	canFly_ = status_.canFly;
	team_ = Team::Enemy;

	model_ = std::move(model);
	model_->SetTranslate(pos);
	transform_.translate = pos;

	shadowModel_ = std::move(shadowModel);
	auto matData = shadowModel_->GetMaterial(0)->GetData();
	matData.color = { 0,0,0,1 };
	shadowModel_->GetMaterial(0)->SetData(matData);

	weapons_ = std::move(rWeapons);
	if (!weapons_.empty()) {
		currentWeapon_ = weapons_[0].get();
	}

	// 初期State
	currentState_ = std::make_unique<EnemyIdle>();

	// 出現演出の間動かない
	appierTimer_.Start(kAppierTime);

	weight_ = 50;

	// ヒット範囲表示
	hitboxDisplay_ = std::make_unique<HitboxDisplay>(team_);

}

void Enemy::Update(MapCheck* mapCheck, Player* player, BulletManager* bulletManager) {
	mapCheck_ = mapCheck;
	// 出現完了までは動かない
	if (appierTimer_.GetState() == TimerState::Running) {
		appierTimer_.Update();

		if (appierTimer_.GetState() == TimerState::Finished) {
			transform_.translate.y = 0;
			appierTimer_.Reset();
		} else {
			return;
		}
	}

	prePos_ = transform_.translate;

	if (hitColorTimer_.GetState() == TimerState::Running) {
		hitColorTimer_.Update();
		if (hitColorTimer_.GetState() == TimerState::Finished) {

			// 色を元に戻す
			auto mat = model_->GetMaterials();
			auto& defaultMat = model_->GetData()->defaultMaterials_;
			for (int i = 0; i < mat.size(); ++i) {
				mat[i]->SetData(defaultMat[i]->GetData());
			}
			hitColorTimer_.Reset();
		}
	}
	slowTimer_.Update();

	// 落下
	if (isFall_) {
		auto* activeReaction = reaction_.get();
		if (activeReaction && activeReaction->Update(this) && reaction_.get() == activeReaction) reaction_.reset();
		return;
	}

	if (reaction_) {
		Stun(mapCheck);
		auto* activeReaction = reaction_.get();
		if (activeReaction && activeReaction->Update(this) && reaction_.get() == activeReaction) reaction_.reset();
		return;
	}

	auto& ctx = GameContext::GetInstance();

	// 現在Stateの行動
	currentState_->Update(this, player, mapCheck, bulletManager);
	velocity_ = currentState_->GetVelocity() * ctx.GetDeltatime();
	if (nextState_) {
		currentState_ = std::move(nextState_);
		nextState_ = nullptr;
	}

	if (slowTimer_.GetState() == TimerState::Running) {
		velocity_ *= 0.4f;
	}

	// 速度をもとに移動
	Vector2 pos = { transform_.translate.x, transform_.translate.z };
	pos.x += velocity_.x;
	mapCheck->ResolveCollisionX(pos, status_.radius, status_.canFly);
	pos.y += velocity_.z;
	mapCheck->ResolveCollisionY(pos, status_.radius, status_.canFly);

	transform_.translate.x = pos.x;
	transform_.translate.z = pos.y;

	if (currentWeapon_) {
		currentWeapon_->Update(transform_.translate, attackDir_, bulletManager, this);
	}
}

void Enemy::Attack(BulletManager* bulletManager) {
	// 武器なし/クールタイム中
	if (!currentWeapon_ || attackCoolTimer_.GetState() == TimerState::Running) {
		return;
	}

	// 攻撃してクールタイム設定
	float time = currentWeapon_->Trigger(model_->GetTransform().translate, attackDir_, bulletManager, this);
	attackCoolTimer_.Start(time);
}

void Enemy::Stun(MapCheck* mapCheck) {
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
		pos.y += knockbackVelocity_.y / 3.0f;
		mapCheck->ResolveCollisionY(pos, status_.radius, true);
	}

	transform_.translate = { pos.x,model_->GetTransform().translate.y,pos.y };
}

void Enemy::OnStunEnd() {
	if (!status_.canFly && mapCheck_ && mapCheck_->IsFall({ transform_.translate.x, transform_.translate.z })) {
		GameContext::GetInstance().Audio().SoundPlay(L"Resources/Sounds/SE/fall.mp3", false);
		isFall_ = true;
		Character::Fall();
	}
}

void Enemy::OnFallEnd() {
	if (transform_.translate.y < -10.0f) isDead_ = true;
	isFall_ = false;
}

void Enemy::DrawOutline() {
	auto& render = GameContext::GetInstance().Render();

	// Drawで設定したトランスフォーム(出現演出を含む)のまま、本体だけ描画
	render.DrawModel(model_.get());
}

void Enemy::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	model_->SetTransform(transform_);
	model_->Update();
	// 影描画
	if (!isFall_) {
		Transform shadowTransform = model_->GetTransform();
		shadowTransform.scale.y = 0.0f;
		shadowTransform.translate.y = 0.01f;
		shadowModel_->SetTransform(shadowTransform);
		render.DrawModel(shadowModel_.get());
	}

	if (appierTimer_.GetState() == TimerState::Running) {
		float r = appierTimer_.GetRemaining() / kAppierTime;
		Vector3 t = model_->GetTransform().translate;
		t.y = -r * r * r * 1.5f;
		model_->SetTranslate(t);
	}
	render.DrawModel(model_.get());

	// 当たり判定表示
	hitboxDisplay_->Update(radius_, ToXZ(transform_.translate));
	hitboxDisplay_->Draw();
}

void Enemy::Hit(float damage, const Vector2& from, const float knockback) {

	status_.hp -= damage;
	if (status_.hp <= 0) { isDead_ = true; }

	if (status_.stunResist < 10) {
		// 行動不能
		Character::Stun(float(10 - status_.stunResist) / 60);

		// ノックバック
		knockbackVelocity_ = Normalize(ToXZ(model_->GetTransform().translate) - from) * knockback;
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
	hitColorTimer_.Start(0.2f);
}
