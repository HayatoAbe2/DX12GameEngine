#include "Player.h"
#include "Objects/Character/Player/PlayerActionState/Dodge/PlayerDodge.h"
#include "Objects/Bullet/BulletManager.h"
#include "Map/MapCheck.h"
#include "Objects/Item/ItemManager.h"
#include "Objects/Weapon/Weapon.h"
#include "Objects/Weapon/WeaponStatus.h"

#include "Objects/Character/Enemy/EnemyManager/EnemyManager.h"
#include "Objects/Item/Passive/Counter/Counter.h"
#include "Objects/Item/Passive/Reload/ReloadBoost.h"
#include "Objects/Item/Passive/Lightning/Lightning.h"
#include "UI/Money/MoneyUI.h"
#include "Controller.h"

Player::~Player() {

}

void Player::Stop() {
	actionState_.reset();
	isUsingBoost_ = false;
}

void Player::Initialize(ItemManager* itemManager) {
	itemManager_ = itemManager;
	team_ = Team::Player;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& light = ctx.Light();

	// ライト
	lightNumber_ = light.AddPointLight();
	PointLight& pointLight = light.GetPointLight(lightNumber_);
	pointLight.decay = 1.0f;
	pointLight.intensity = 0.5f;

	model_ = asset.LoadModel("Resources/Debug/human", "walk.gltf");
	auto matData = model_->GetMaterial(0)->GetData();
	matData.color = { 0.5f,0.5f,1,1 };
	matData.useEnvironmentMap = true;
	matData.environmentIntensity = 1.0f;
	model_->GetMaterial(0)->SetData(matData);

	shadowModel_ = asset.LoadModel("Resources/Debug/human", "walk.gltf");
	matData = shadowModel_->GetMaterial(0)->GetData();
	matData.color = { 0,0,0,1 };
	shadowModel_->GetMaterial(0)->SetData(matData);

	walk_ = asset.LoadAnimation("Resources/Debug/human", "walk.gltf")[0];
	walk_->isLoop = true;
	sneak_ = asset.LoadAnimation("Resources/Debug/human", "sneakWalk.gltf")[0];
	sneak_->isLoop = true;
	model_->SetAnimation(walk_);

	// 残像
	for (int i = 0; i < 2; ++i) {
		instancing_[i] = asset.LoadModel("Resources/Debug/human", "walk.gltf");
		instancing_[i]->SetAnimation(asset.LoadAnimation("Resources/Debug/human", "walk.gltf")[0]);
		MaterialData data = instancing_[i]->GetMaterial(0)->GetData();
		data.color = { 0.5f,0.5f,0.5f, 1.0f - (i + 1) * 0.25f };
		data.enableLighting = false;
		instancing_[i]->GetMaterial(0)->SetData(data);
	}

	// 方向線
	direction_ = asset.LoadModel("Resources/Models/Direction", "Direction.obj");
	auto dData = direction_->GetMaterial(0)->GetData();
	dData.color = { 1,0,0,dirDisplayAlpha_ };
	direction_->GetMaterial(0)->SetData(dData);

	// 移動時パーティクル
	moveParticle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), moveParticleNum_);
	moveParticle_->SetLifeTime(10);
	moveParticle_->SetColor({ 0.6f, 0.6f, 0.6f, 1.0f });

	shootCooldownSprite_ = asset.LoadSprite("Resources/Debug/White1x1.png");
	shootCooldownSprite_->SetPivot({ 0.0f,0.5f });
	shootCooldownSprite_->SetSize(scSize_);
	Vector2 windowSize = ctx.GetRenderWindowSize();
	shootCooldownSprite_->SetPosition({ (windowSize.x - scSize_.x) / 2.0f, windowSize.y / 2.0f - scSize_.y * 2.5f });

	attackDirection_ = Normalize(Vector2(1, 0));
	transform_.rotate.y = -std::atan2(attackDirection_.y, attackDirection_.x) + std::numbers::pi_v<float> / 2.0f;

	// ヒット範囲表示
	hitboxDisplay_ = std::make_unique<HitboxDisplay>(team_);
}

void Player::Update(MapCheck* mapCheck, Camera* camera, BulletManager* bulletManager) {
	mapCheck_ = mapCheck;
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& input = ctx.Input();
	auto& light = ctx.Light();

	prePos_ = transform_.translate;
	model_->Update();
	Vector2 d = attackDirection_ * directionFactor;
	light.GetPointLight(lightNumber_).position = prePos_ + Vector3{ d.x, kLightHeight, d.y };

	invincibleTimer_.Update();
	if (invincibleTimer_.IsActive() && !isUsingBoost_) {
		float t = invincibleTimer_.GetRemaining() * 10.0f;
		float strength = (std::sin(t) + 1.0f) * 0.5f; // 0~1
		float c = Lerp(1.0f, 2.0f, strength);
		for (auto& mat : model_->GetMaterials()) {
			MaterialData matD = mat->GetData();
			matD.color = Vector4{ c,c,c,2.5f - c };
			mat->SetData(matD);
		}
	} else {
		for (auto& mat : model_->GetMaterials()) {
			MaterialData matD = mat->GetData();
			matD.color = Vector4{ 1,1,1,1 };
			mat->SetData(matD);
		}
	}

	if (hitColorTimer_.IsActive()) {
		hitColorTimer_.Update();

		if (hitColorTimer_.GetState() == TimerState::Finished) {
			for (auto& mesh : model_->GetData()->meshes) {
				auto data = model_->GetMaterial(0)->GetData();
				data.color = { 1.0f,1.0f,1.0f,1.0f };
				model_->GetMaterial(0)->SetData(data);
			}
		}
	}

	if (isFall_) {
		auto* activeReaction = reaction_.get();
		if (activeReaction && activeReaction->Update(this) && reaction_.get() == activeReaction) reaction_.reset();
	} else {
		if (actionState_) {
			auto* activeState = actionState_.get();
			if (activeState->Update(this, mapCheck) && actionState_.get() == activeState) actionState_.reset();
		} else {
			Move(mapCheck);

			// 行動Stateの処理
			if (actionState_) {
				auto* activeState = actionState_.get();
				if (activeState->Update(this, mapCheck) && actionState_.get() == activeState) actionState_.reset();
			}
		}

		for (auto& p : passives_) {
			p->OnUpdate(weapon_.get(), subWeapon_.get());
		}

		// アイテム取得
		if (input.keyboard.IsRelease(DIK_F) || input.gamepad.IsRelease(XINPUT_GAMEPAD_A)) {
			itemManager_->Interact(this);
		}

		// 攻撃の向き
		if (input.gamepad.IsConnected()) {
			if (Length(input.gamepad.GetRightStick()) > 0.3f) {
				Vector2 dir;
				dir.x = input.gamepad.GetRightStick().x;
				dir.y = input.gamepad.GetRightStick().y;

				attackDirection_ = Normalize(Vector2(dir.x, dir.y));
			}
		} else {
			Vector2 win = ctx.GetWindowSize();
			Vector2 mouse = input.mouse.GetPosition();
			mouse.y = win.y - mouse.y;

			Vector2 dir = Normalize(mouse - win / 2.0f);
			attackDirection_ = { dir.x,dir.y };
		}
		// プレイヤーの向き
		transform_.rotate.y = -std::atan2(attackDirection_.y, attackDirection_.x) + std::numbers::pi_v<float> / 2.0f;

		// 減速
		if (weapon_) {
			float weight = weapon_->GetData().stats.weight;
			moveSpeed_ = defaultMoveSpeed_ * (1.0f - weight);
		} else {
			moveSpeed_ = defaultMoveSpeed_;
		}

		if (weapon_) {


			if (input.mouse.IsPress(MouseButton::Left) || input.gamepad.GetRTrigger() > kTriggerThreshold) {
				weapon_->Update(transform_.translate, attackDirection_, bulletManager, this, true);
			} else {
				weapon_->Update(transform_.translate, attackDirection_, bulletManager, this, false);
			}

			if (subWeapon_) {
				subWeapon_->Update(transform_.translate, attackDirection_, bulletManager, this);

				// 入れ替え
				if ((input.keyboard.IsTrigger(DIK_TAB) || input.gamepad.IsTrigger(XINPUT_GAMEPAD_B)) && weapon_->CanChange()) {
					weapon_.swap(subWeapon_);
				}
			}

			// 武器のトランスフォーム
			weaponTransform_ = transform_;
			weaponTransform_.translate += {std::sin(transform_.rotate.y), 0.5f, std::cos(transform_.rotate.y)}; // 前方に配置

			if (!shootCooldownTimer_.IsActive()) {
				// 射撃
#ifdef USE_IMGUI
				if (ctx.IsSceneWindowHovered()) {
#endif
					if (input.mouse.IsPress(MouseButton::Left)) {
						Trigger(bulletManager, camera);
					}
#ifdef USE_IMGUI
				}
#endif
				if (input.gamepad.GetRTrigger() > 0.2f) {
					Trigger(bulletManager, camera);
				}

			} else {
				shootCooldownTimer_.Update();
			}

			const float startTime = shootCooldownTimer_.GetStartTime();
			const float cooldownProgress = startTime > 0.0f
				? 1.0f - shootCooldownTimer_.GetRemaining() / startTime
				: 1.0f;
			shootCooldownSprite_->SetSize({ scSize_.x * cooldownProgress, scSize_.y });
		}

		if (reaction_) {
			Stun(mapCheck);
			auto* activeReaction = reaction_.get();
			if (activeReaction && activeReaction->Update(this) && reaction_.get() == activeReaction) reaction_.reset();
		}
	}

	moveParticle_->Update();

	// 残像
	instancingTransforms[3] = instancingTransforms[2];
	instancingTransforms[2] = instancingTransforms[1];
	instancingTransforms[1] = instancingTransforms[0];
	instancingTransforms[0] = transform_;

	instancing_[0]->SetTransform(instancingTransforms[1]);
	instancing_[1]->SetTransform(instancingTransforms[2]);
	for (int i = 0; i < 2; ++i) {
		instancing_[i]->Update();
	}

	if (hp_ <= 0.0f) {
		isDead_ = true;
	}
}

void Player::DrawOutline() {
	auto& render = GameContext::GetInstance().Render();

	// Drawで設定したトランスフォームのまま、本体だけ描画
	render.DrawModel(model_.get());
}

void Player::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	// 影描画
	Transform shadowTransform;
	if (!isFall_) {
		shadowTransform = transform_;
		shadowTransform.scale.y = 0.0f;
		shadowTransform.translate.y = 0.01f;
		shadowModel_->SetTransform(shadowTransform);
		render.DrawModel(shadowModel_.get());
	}
	if (isUsingBoost_) {
		for (int i = 0; i < 2; ++i) {
			render.AddPostEffect(PostEffectType::BoxFilter5x5);
			render.AddPostEffect(PostEffectType::GaussianFilter3x3);
			render.DrawModel(instancing_[i].get());
		}
	}

	for (auto& p : passives_) {
		p->Draw();
	}

	model_->SetTransform(transform_);
	render.DrawModel(model_.get());

	if (weapon_) {
		// 影描画
		shadowTransform = weaponTransform_;
		shadowTransform.scale.y = 0.0f;
		shadowTransform.translate.y = 0.01f;
		weapon_->GetShadowModel()->SetTransform(shadowTransform);
		render.DrawModel(weapon_->GetShadowModel());

		// 武器描画
		weapon_->Draw();
		weapon_->GetModel()->SetTransform(weaponTransform_);
		render.DrawModel(weapon_->GetModel());

		// 照準方向
		direction_->SetTransform(weaponTransform_);
		render.DrawModel(direction_.get());

		// クールダウン表示
		if (shootCooldownTimer_.IsActive()) {
			render.DrawSprite(shootCooldownSprite_.get());
		}
	}

	// パーティクル
	render.DrawParticle(moveParticle_.get(), BlendMode::Add);

	// 当たり判定表示
	hitboxDisplay_->Update(radius_, ToXZ(transform_.translate));
	hitboxDisplay_->Draw();

#ifdef USE_IMGUI
	ImGui::Begin("Player Info");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform_.translate.x, transform_.translate.y, transform_.translate.z);
	ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform_.rotate.x, transform_.rotate.y, transform_.rotate.z);
	ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform_.scale.x, transform_.scale.y, transform_.scale.z);
	ImGui::Text("HP: %f", hp_);

	ImGui::End();
#endif
}

void Player::Hit(float damage, const Vector2& from) {
	if (!invincibleTimer_.IsActive()) {

		if (damage > 0) {
			hp_ -= damage;
			invincibleTimer_.Start(invincibleTimeOnHit_);

			// 行動不能
			Character::Stun(stunTime_);

			// ノックバック
			knockbackVel_ = Normalize(ToXZ(model_->GetTransform().translate) - from) * 20.0f;

			// ダメージを受けると色変更
			auto data = model_->GetMaterial(0)->GetData();
			data.color = { 3.0f,3.0f,3.0f,1.0f };
			model_->GetMaterial(0)->SetData(data);
			hitColorTimer_.Start(0.2f);

			// 被ダメージ時の位置を記憶
			landPos_ = transform_.translate;
		}
	}
}

void Player::OnDealDamage(const Vector2& pos) {
	for (auto& p : passives_) {
		p->OnDealDamage(pos);
	}
}
void Player::OnEliminate(const Vector2& enemyPos) {
	for (auto& p : passives_) {
		p->OnEliminate(enemyPos, this);
	}
}
void Player::OnHit(const Vector2& from) {
	for (auto& p : passives_) {
		p->OnHit(from, this);
	}
}
void Player::OnDodge() {
	for (auto& p : passives_) {
		p->OnDodge(this);
	}
}

void Player::Move(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	// 入力方向
	Vector2 dir = { 0,0 };

	// 移動
	if (Length(input.gamepad.GetLeftStick()) > kStickDeadzone) {
		dir.x = input.gamepad.GetLeftStick().x;
		dir.y = -input.gamepad.GetLeftStick().y;
	}

	if (input.keyboard.IsPress(DIK_A)) {
		dir.x = -1;
	}

	if (input.keyboard.IsPress(DIK_D)) {
		dir.x = 1;
	}

	if (input.keyboard.IsPress(DIK_W)) {
		dir.y = -1;
	}

	if (input.keyboard.IsPress(DIK_S)) {
		dir.y = 1;
	}

	if (input.mouse.IsPress(MouseButton::Right) || input.gamepad.GetLTrigger() > kTriggerThreshold) {
		model_->ChangeAnimation(sneak_, 0.15f);
		moveSpeed_ /= 2.0f;
		for (int i = 0; i < 2; ++i) {
			instancing_[i]->ChangeAnimation(sneak_, 0.15f);
		}
	} else {
		model_->ChangeAnimation(walk_, 0.15f);
		for (int i = 0; i < 2; ++i) {
			instancing_[i]->ChangeAnimation(walk_, 0.15f);
		}
	}

	// 入力を反映
	Vector2 normalized = Normalize(dir);
	velocity_.x = normalized.x * moveSpeed_ * ctx.GetDeltatime();
	velocity_.z = -normalized.y * moveSpeed_ * ctx.GetDeltatime();

	boostCoolTime_--;

	// ダッシュ入力
	if ((input.keyboard.IsTrigger(DIK_SPACE) || input.gamepad.IsTrigger(XINPUT_GAMEPAD_X)) && Length(normalized) > 0.1f && boostCoolTime_ < 0) {
		isUsingBoost_ = true;
		Vector3 dodgeDirection = { normalized.x,0,-normalized.y };
		actionState_ = std::make_unique<PlayerDodge>(dodgeDirection, boostSpeed_, maxBoostTime_);
		// ダッシュ前の位置を記憶
		landPos_ = transform_.translate;

		for (int i = 0; i < 5; ++i) {
			Vector3 randomVector = {
			ctx.RandomFloat(-moveParticleRange_ / 2.0f, moveParticleRange_ / 2.0f),
			-0.5f,
			ctx.RandomFloat(-moveParticleRange_ / 2.0f, moveParticleRange_ / 2.0f),
			};
			Transform transform = model_->GetTransform();
			transform.translate += randomVector;
			transform.scale = { 1.0f,1.0f,1.0f };
			moveParticle_->Emit(transform, -velocity_ * 0.4f);
		}
		moveParticleEmitTimer_ = 0;

		invincibleTimer_.Start(invincibleTimeOnDodge_);
		return;
	}

	// 速度をもとに移動
	Vector2 pos = { transform_.translate.x,transform_.translate.z };
	for (int i = 0; i < 4; ++i) {
		pos.x += velocity_.x / 4.0f;
		mapCheck->ResolveCollisionX(pos, radius_, isUsingBoost_);
		pos.y += velocity_.z / 4.0f;
		mapCheck->ResolveCollisionY(pos, radius_, isUsingBoost_);
	}
	transform_.translate.x = pos.x;
	transform_.translate.z = pos.y;

	if (velocity_.x != 0 || velocity_.z != 0) {
		// 移動による向き変更
		transform_.rotate.y = -std::atan2(velocity_.z, velocity_.x) + std::numbers::pi_v<float> / 2.0f;

		// パーティクル
		moveParticleEmitTimer_++;
		if (moveParticleEmitTimer_ >= moveParticleEmitInterval_) {
			for (int i = 0; i < 3; ++i) {
				Vector3 randomVector = {
				ctx.RandomFloat(-moveParticleRange_ / 2.0f, moveParticleRange_ / 2.0f),
				-0.5f,
				ctx.RandomFloat(-moveParticleRange_ / 2.0f, moveParticleRange_ / 2.0f),
				};
				Transform transform = model_->GetTransform();
				transform.translate += randomVector;
				transform.scale = { 1.0f,1.0f,1.0f };
				moveParticle_->Emit(transform, -velocity_ * 0.4f);
			}
			moveParticleEmitTimer_ = 0;
		}
	}
}

void Player::Trigger(BulletManager* bulletManager, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	if (weapon_) {
		shootCooldownTimer_.Start(weapon_->Trigger(weaponTransform_.translate, attackDirection_, bulletManager, this));
	}
}

void Player::OnFallEnd() {
	if (transform_.translate.y < -20.0f) {
		hp_ -= maxHp_ / 8.0f;
		invincibleTimer_.Start(invincibleTimeOnHit_);

		// その前にいた位置に戻す
		transform_.translate = landPos_;
	}
	isFall_ = false;
}

void Player::OnStunEnd() {
	if (mapCheck_ && mapCheck_->IsFall({ transform_.translate.x, transform_.translate.z })) {
		GameContext::GetInstance().Audio().SoundPlay(L"Resources/Sounds/SE/fall.mp3", false);
		isFall_ = true;
		Character::Fall();
	}
}

void Player::Stun(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();

	if (reaction_) {

		// ノックバック
		model_->SetScale({ 1,1,1 });
		float length = Length(knockbackVel_) * ctx.GetDeltatime();
		float x = GetReactionRemaining() / stunTime_;
		length *= x * x;
		if (length < 0) { length = 0; }
		knockbackVel_ = Normalize(knockbackVel_) * length;

		// 速度をもとに移動
		Vector2 pos = { transform_.translate.x,transform_.translate.z };
		for (int i = 0; i < 3; ++i) { // 3回に分ける
			pos.x += knockbackVel_.x / 3.0f;
			mapCheck->ResolveCollisionX(pos, radius_, true);
			pos.y += knockbackVel_.y / 3.0f;
			mapCheck->ResolveCollisionY(pos, radius_, true);
		}

		transform_.translate = { pos.x, model_->GetTransform().translate.y, pos.y };
	}
}

void Player::SetWeapon(std::unique_ptr<Weapon> weapon) {
	// サブ武器に装備
	if (weapon_ && subWeapon_ == nullptr) {
		subWeapon_ = std::move(weapon);
		return;
	}

	// 両方持っているなら交換
	if (weapon_ && subWeapon_) {
		itemManager_->Drop(transform_.translate, std::move(weapon_));
	}

	// 装備
	weapon_ = std::move(weapon);
}

void Player::AddPassive(std::unique_ptr<Passive> passive) {
	Sprite* sprite = passive->GetSprite();
	int index = int(passives_.size());
	sprite->SetPosition({ 500.0f + 80 * index, sprite->GetSize().y / 2.0f }); // 画面上位置の押し出し

	passives_.push_back(std::move(passive));
}
