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
#include <Character/Enemy/EnemyManager.h>
#include <Item/Passive/Counter/Counter.h>
#include <Item/Passive/Reload/ReloadBoost.h>
#include <Item/Passive/Lightning/Lightning.h>
#include "UI/Money/MoneyUI.h"

Player::~Player() {

}

void Player::Initialize(std::unique_ptr<Model> playerModel, std::unique_ptr<Model> playerShadow, ItemManager* itemManager) {
	itemManager_ = itemManager;

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
	for (int i = 0; i < 2; ++i) {
		instancing_[i] = asset.LoadModel("Resources/Debug/human", "walk.gltf");
		instancing_[i]->SetAnimation(asset.LoadAnimation("Resources/Debug/human", "walk.gltf"));
		MaterialData data = instancing_[i]->GetMaterial(0)->GetData();
		data.color = { 0.5f,0.5f,0.5f, 1.0f - (i + 1) * 0.25f };
		data.enableLighting = false;
		instancing_[i]->GetMaterial(0)->SetData(data);
	}

	// 方向線
	direction_ = asset.LoadModel("Resources/Direction", "Direction.obj");
	auto dData = direction_->GetMaterial(0)->GetData();
	dData.color = { 1,0,0,dirDisplayAlpha_ };
	direction_->GetMaterial(0)->SetData(dData);

	// 移動時パーティクル
	moveParticle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Fire/circle.png")), moveParticleNum_);
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

	attackDirection_ = Normalize(Vector2(1, 0));
	transform_.rotate.y = -std::atan2(attackDirection_.y, attackDirection_.x) + float(std::numbers::pi) / 2.0f;
	transform_.translate.x = 5;
}

void Player::Update(MapCheck* mapCheck, Camera* camera, BulletManager* bulletManager) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& input = ctx.Input();

	prePos_ = model_->GetTransform().translate;
	model_->Update();

	invincibleTimer_->Update();
	if (invincibleTimer_->IsActive() && !isUsingBoost_) {
		float t = invincibleTimer_->GetRemaining() * 10.0f;
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
		transform_.rotate.y = -std::atan2(attackDirection_.y, attackDirection_.x) + float(std::numbers::pi) / 2.0f;

		// 減速
		if (weapon_) {
			float weight = weapon_->GetData().stats.weight;
			moveSpeed_ = defaultMoveSpeed_ * (1.0f - weight);
		} else {
			moveSpeed_ = defaultMoveSpeed_;
		}

		if (weapon_) {


			if (input.mouse.IsPress(MouseButton::Left) || input.gamepad.GetRTrigger() > 0.2f) {
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

			if (shootCooldownTimer_->IsFinished()) {
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
				shootCooldownTimer_->Update();
			}

			shootCooldownSprite_->SetSize({ scSize_.x * (1.0f - shootCooldownTimer_->GetRemaining() / shootCooldownTimer_->GetStartTime()), scSize_.y });
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

	instancing_[0]->SetTransform(instancingTransforms[1]);
	instancing_[1]->SetTransform(instancingTransforms[2]);
	for (int i = 0; i < 2; ++i) {
		instancing_[i]->Update();
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
	}
	if (isUsingBoost_) {
		for (int i = 0; i < 2; ++i) {
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
		weapon_->GetModel()->SetTransform(weaponTransform_);
		render.DrawModel(weapon_->GetModel());

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

void Player::Hit(float damage, const Vector2& from) {
	auto& ctx = GameContext::GetInstance();

	if (invincibleTimer_->IsFinished()) {

		if (damage > 0) {
			hp_ -= damage;
			invincibleTimer_->Start(invincibleTimeOnHit_);

			// 行動不能
			stunTimer_->Start(stunTime_);

			// ノックバック
			knockbackVel_ = Normalize(ToXZ(model_->GetTransform().translate) - from) * 20.0f;

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

void Player::OnHit(const Vector2& from, BulletManager* bulletManager) {
	for (auto& p : passives_) {
		p->OnHit(from, bulletManager, this);
	}
}

void Player::OnDealDamage(const Vector2& pos, EnemyManager* enemyManager) {
	for (auto& p : passives_) {
		p->OnDealDamage(pos, enemyManager);
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

void Player::Trigger(BulletManager* bulletManager, Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();

	if (weapon_) {
		shootCooldownTimer_->Start(weapon_->Trigger(weaponTransform_.translate, attackDirection_, bulletManager, this));
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
			pos.y += knockbackVel_.y / 3.0f;
			mapCheck->ResolveCollisionY(pos, radius_, true);
		}

		if (stunTimer_->IsFinished() && mapCheck->IsFall(pos)) {
			audio.SoundPlay(L"Resources/Sounds/SE/fall.mp3", false);
		}

		transform_.translate = { pos.x,model_->GetTransform().translate.y,pos.y };
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
	sprite->SetSize({ 75, 75 });
	sprite->SetPosition({ 500.0f + 55 * index, 0 });

	passives_.push_back(std::move(passive));
}
