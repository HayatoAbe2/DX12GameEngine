#include "Player.h"
#include "Bullet/BulletManager.h"
#include "Map/MapCheck.h"
#include "Item/ItemManager.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponStatus.h"

#include <numbers>
#include <cmath>
#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"


Player::~Player() {

}

void Player::Initialize(std::unique_ptr<Model> playerModel, std::unique_ptr<Model> playerShadow) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	model_ = std::move(playerModel);
	auto matData = model_->GetMaterial(0)->GetData();
	matData.color = { 0.5f,0.5f,1,1 };
	model_->GetMaterial(0)->SetData(matData);
	shadowModel_ = std::move(playerShadow);
	matData = shadowModel_->GetMaterial(0)->GetData();
	matData.color = { 0,0,0,1 };
	shadowModel_->GetMaterial(0)->SetData(matData);
	transform_.translate.x = 1;
	transform_.translate.z = 1;

	// 残像
	instancing_ = asset.LoadInstancedModel("Resources/Debug/human", "walk.gltf", 2);
	MaterialData data = instancing_->GetMaterial(0)->GetData();
	data.color = { 0.3f,0.3f,1,0.2f };
	data.enableLighting = false;
	instancing_->GetMaterial(0)->SetData(data);

	// 方向線
	direction_ = asset.LoadModel("Resources/Direction", "Direction.obj");
	auto dData = direction_->GetMaterial(0)->GetData();
	dData.color = { 1,0,0,dirDisplayAlpha_ };
	direction_->GetMaterial(0)->SetData(data);

	// 移動時パーティクル
	moveParticle_ = asset.CreateParticleSystem(ParticleShape::Plane, asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), moveParticleNum_);
	moveParticle_->SetLifeTime(10);
	moveParticle_->SetColor({ 0.6f, 0.6f, 0.6f, 1.0f });

	shootCooldownTimer_ = std::make_unique<Timer>();
	shootCooldownSprite_ = asset.LoadSprite("Resources/Debug/White1x1.png");
	shootCooldownSprite_->SetPivot({ 0.0f,0.5f });
	shootCooldownSprite_->SetSize(scSize_);
	shootCooldownSprite_->SetPosition({ 640 - scSize_.x / 2.0f,370 });

	// タイマー類
	hitColorTimer_ = std::make_unique<Timer>();
	stunTimer_ = std::make_unique<Timer>();
	boostTimer_ = std::make_unique<Timer>();
	invincibleTimer_ = std::make_unique<Timer>();

	attackDirection_ = Normalize(Vector3(1, 0, 0));
	transform_.rotate.y = -std::atan2(attackDirection_.z, attackDirection_.x) + float(std::numbers::pi) / 2.0f;
}

void Player::Update(MapCheck* mapCheck, ItemManager* itemManager, Camera* camera, BulletManager* bulletManager) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& input = ctx.Input();

	model_->Update();

	invincibleTimer_->Update();
	if (hitColorTimer_->IsActive()) {
		hitColorTimer_->Update();

		if (hitColorTimer_->IsFinished()) {
			for (auto& mesh : model_->GetData()->meshes) {
				auto data = model_->GetMaterial(0)->GetData();
				data.color = { 1.0f,1.0f,1.0f,1.0f };
				model_->GetMaterial(0)->SetData(data);
			}
		}
	}

	if (isFall_) {
		Fall();
	} else {
		if (isUsingBoost_) {
			Boost(mapCheck);
		} else {
			Move(mapCheck);
		}

		// アイテム取得
		if (input.keyboard.IsRelease(DIK_F) || input.gamepad.IsRelease(XINPUT_GAMEPAD_A)) {
			itemManager->Interact(this);
		}

		// 攻撃の向き
		if (input.gamepad.IsConnected()) {
			if (Length(input.gamepad.GetRightStick()) > 0.3f) {
				Vector2 dir;
				dir.x = input.gamepad.GetRightStick().x;
				dir.y = input.gamepad.GetRightStick().y;

				attackDirection_ = Normalize(Vector3(dir.x, 0, dir.y));
			}
		} else {
			Vector2 win = ctx.GetWindowSize();
			Vector2 mouse = input.mouse.GetPosition();
			mouse.y = win.y - mouse.y;

			Vector2 dir = Normalize(mouse - win / 2.0f);
			attackDirection_ = { dir.x,0,dir.y };
		}
		// プレイヤーの向き
		transform_.rotate.y = -std::atan2(attackDirection_.z, attackDirection_.x) + float(std::numbers::pi) / 2.0f;

		// 減速
		if (weapon_) {
			float weight = weapon_->GetData().stats.weight + weapon_->GetModifier().add.weight;
			weight *= weapon_->GetModifier().multiplier.weight;

			moveSpeed_ = defaultMoveSpeed_ * (1.0f - weight);
		} else {
			moveSpeed_ = defaultMoveSpeed_;
		}

		if (weapon_) {
			weapon_->Update();

			// 入れ替え
			if ((input.keyboard.IsTrigger(DIK_TAB) || input.gamepad.IsTrigger(XINPUT_GAMEPAD_B)) && subWeapon_) {
				weapon_.swap(subWeapon_);
			}

			// 武器のトランスフォーム
			weaponTransform_ = transform_;
			weaponTransform_.translate += {std::sin(transform_.rotate.y), 0.5f, std::cos(transform_.rotate.y)}; // 前方に配置

			if (shootCooldownTimer_->IsFinished()) {
				// 射撃
#ifdef USE_IMGUI
				if (ctx.IsSceneWindowHovered()) {
#endif
					if (input.mouse.IsPress(MouseButton::Left)) {
						Shoot(bulletManager, camera);
					}
#ifdef USE_IMGUI
				}
#endif
				if (input.gamepad.GetRTrigger() > 0.2f) {
					Shoot(bulletManager, camera);
				}

			} else {
				shootCooldownTimer_->Update();
			}

			shootCooldownSprite_->SetSize({ scSize_.x * (1.0f - shootCooldownTimer_->GetRemaining() / shootCooldownTimer_->GetStartTime()), scSize_.y });
		}

		if (stunTimer_->IsActive()) {
			Stun(mapCheck);
		}
	}

	moveParticle_->Update();

	// 残像
	instancingTransforms[3] = instancingTransforms[2];
	instancingTransforms[2] = instancingTransforms[1];
	instancingTransforms[1] = instancingTransforms[0];
	instancingTransforms[0] = transform_;

	for (int i = 0; i < 2; ++i) {
		instancing_->SetInstanceTransforms(i, instancingTransforms[i]);
	}
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
	} else {
		render.SetPostEffectType(PostEffectType::BoxFilter5x5);
	}

	if (isUsingBoost_) {
		render.DrawInstancedModel(instancing_.get(), BlendMode::Add);
	}

	model_->SetTransform(transform_);
	render.DrawModel(model_.get());

	if (weapon_) {
		// 影描画
		shadowTransform = weaponTransform_;
		shadowTransform.scale.y = 0.0f;
		shadowTransform.translate.y = 0.01f;
		weapon_->GetWeaponShadowModel()->SetTransform(shadowTransform);
		render.DrawModel(weapon_->GetWeaponShadowModel());

		// 武器描画
		weapon_->GetWeaponModel()->SetTransform(weaponTransform_);
		render.DrawModel(weapon_->GetWeaponModel());

		// 照準方向
		direction_->SetTransform(weaponTransform_);
		render.DrawModel(direction_.get());

		// クールダウン表示
		if (shootCooldownTimer_->IsActive()) {
			render.DrawSprite(shootCooldownSprite_.get());
		}
	}

	// パーティクル
	render.DrawParticle(moveParticle_.get(), BlendMode::Add);

#ifdef USE_IMGUI
	ImGui::Begin("Player Info");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", transform_.translate.x, transform_.translate.y, transform_.translate.z);
	ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", transform_.rotate.x, transform_.rotate.y, transform_.rotate.z);
	ImGui::Text("Scale: (%.2f, %.2f, %.2f)", transform_.scale.x, transform_.scale.y, transform_.scale.z);
	ImGui::Text("HP: %f", hp_);
	ImGui::End();
#endif
}

void Player::Hit(float damage, Vector3 from) {
	auto& ctx = GameContext::GetInstance();

	if (invincibleTimer_->IsFinished()) {

		if (damage > 0) {
			hp_ -= damage;
			invincibleTimer_->Start(invincibleTimeOnHit_);

			if (hp_ <= 0) {
				// ゲームオーバー

			} else {
				// 行動不能
				stunTimer_->Start(stunTime_);

				// ノックバック
				knockbackVel_ = Normalize(model_->GetTransform().translate - from) * 15.0f;

				// ダメージを受けると色変更
				auto data = model_->GetMaterial(0)->GetData();
				data.color = { 3.0f,3.0f,3.0f,1.0f };
				model_->GetMaterial(0)->SetData(data);
				hitColorTimer_->Start(0.2f);

				// 被ダメージ時の位置を記憶
				landPos_ = transform_.translate;
			}
		}
	}
}

void Player::Move(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	// 入力方向
	Vector2 dir = { 0,0 };

	// 移動
	if (Length(input.gamepad.GetLeftStick()) > 0.3f) {
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

	// 入力を反映
	Vector2 normalized = Normalize(dir);
	velocity_.x = normalized.x * moveSpeed_ * ctx.GetDeltatime();
	velocity_.z = -normalized.y * moveSpeed_ * ctx.GetDeltatime();

	boostCoolTime_--;

	// ダッシュ入力
	if ((input.keyboard.IsTrigger(DIK_SPACE) || input.gamepad.IsTrigger(XINPUT_GAMEPAD_X)) && Length(normalized) > 0.1f && boostCoolTime_ < 0) {
		isUsingBoost_ = true;
		boostTimer_->Start(maxBoostTime_);
		boostDir_ = { normalized.x,0,-normalized.y };
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

		invincibleTimer_->Start(invincibleTimeOnDodge_);
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
		transform_.rotate.y = -std::atan2(velocity_.z, velocity_.x) + float(std::numbers::pi) / 2.0f;

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

void Player::Shoot(BulletManager* bulletManager, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	if (weapon_) {
		shootCooldownTimer_->Start(weapon_->Shoot(weaponTransform_.translate, attackDirection_, bulletManager, camera, this));
	}
}

void Player::Boost(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	// ダッシュ
	velocity_ = boostDir_ * boostSpeed_ * ctx.GetDeltatime();

	Vector2 pos = { transform_.translate.x,transform_.translate.z };
	for (int i = 0; i < 4; ++i) {
		pos.x += velocity_.x / 4.0f;
		mapCheck->ResolveCollisionX(pos, radius_, isUsingBoost_);
		pos.y += velocity_.z / 4.0f;
		mapCheck->ResolveCollisionY(pos, radius_, isUsingBoost_);
	}
	transform_.translate.x = pos.x;
	transform_.translate.z = pos.y;

	boostTimer_->Update();

	if (boostTimer_->IsFinished()) { // 終了時
		isUsingBoost_ = false;

		// 落下判定
		if (mapCheck->IsFall({ transform_.translate.x,transform_.translate.z })) {
			isFall_ = true;
			audio.SoundPlay(L"Resources/Sounds/SE/fall.mp3", false);
		}
	}
}

void Player::Fall() {
	// 落下
	transform_.translate.y -= 1.0f;
	if (transform_.translate.y < -20.0f) {
		hp_ -= maxHp_ / 8.0f;
		invincibleTimer_->Start(invincibleTimeOnHit_);

		// その前にいた位置に戻す
		transform_.translate = landPos_;
		isFall_ = false;
		stunTimer_->Reset();
	}
}

void Player::Stun(MapCheck* mapCheck) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();

	if (stunTimer_->IsActive()) {
		stunTimer_->Update();

		// ノックバック
		model_->SetScale({ 1,1,1 });
		float length = Length(knockbackVel_) * ctx.GetDeltatime();
		float x = stunTimer_->GetRemaining() / stunTime_;
		length *= x * x;
		if (length < 0) { length = 0; }
		knockbackVel_ = Normalize(knockbackVel_) * length;

		// 速度をもとに移動
		Vector2 pos = { transform_.translate.x,transform_.translate.z };
		for (int i = 0; i < 3; ++i) { // 3回に分ける
			pos.x += knockbackVel_.x / 3.0f;
			mapCheck->ResolveCollisionX(pos, radius_, true);
			pos.y += knockbackVel_.z / 3.0f;
			mapCheck->ResolveCollisionY(pos, radius_, true);
		}

		if (stunTimer_->IsFinished() && mapCheck->IsFall(pos)) {
			audio.SoundPlay(L"Resources/Sounds/SE/fall.mp3", false);
		}

		transform_.translate = { pos.x,model_->GetTransform().translate.y,pos.y };
	}
}

void Player::SetWeapon(std::unique_ptr<Weapon> weapon) {
	if (weapon_ && subWeapon_ == nullptr) {
		subWeapon_ = std::move(weapon);
		return;
	}

	weapon_ = std::move(weapon);
}