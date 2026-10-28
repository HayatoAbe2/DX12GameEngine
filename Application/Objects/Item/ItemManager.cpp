#include "ItemManager.h"
#include "Objects/Character/Player/Player.h"
#include "Objects/Weapon/WeaponManager/WeaponManager.h"
#include "Objects/Weapon/Weapon.h"
#include "Engine/Scene/BaseScene/BaseScene.h"
#include "Engine/SceneObject/SceneObject.h"
#include "Objects/Item/World/WorldWeapon/WorldWeapon.h"
#include "Objects/Item/World/WorldPassive/WorldPassive.h"
#include "Objects/Item/World/WorldMoney/WorldMoney.h"
#include "Objects/Item/Passive/PassiveItems.h"

void ItemManager::Initialize(WeaponManager* weaponManager, EnemyManager* enemyManager, BulletManager* bulletManager) {
	weaponManager_ = weaponManager;
	enemyManager_ = enemyManager;
	bulletManager_ = bulletManager;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 操作
	controlKey_ = asset.LoadSprite("Resources/Images/Control/KeyboardAndMouse.dds");
	controlKey_->SetSize({ 64,64 });
	controlKey_->SetPosition({ 640 - 32,720 - 460 });
	controlKey_->SetTextureRect(0, 64 * 9, 64, 64);
	controlPad_ = asset.LoadSprite("Resources/Images/Control/XboxController.dds");
	controlPad_->SetSize({ 64,64 });
	controlPad_->SetPosition({ 640 - 32,720 - 460 });
	controlPad_->SetTextureRect(64 * 2, 64 * 8, 64, 64);
}

void ItemManager::Update(Player* player, bool isCombat) {
	// 配置予約があれば出現
	if (nextSpawnIndex_ > 0) {
		Drop(nextSpawnPos_, weaponManager_->GetWeapon(nextSpawnIndex_));
		nextSpawnIndex_ = -1;
	}

	// 削除
	items_.erase(
		std::remove_if(items_.begin(), items_.end(),
			[](const std::unique_ptr<WorldItem>& item) {
				return item->IsDead();
			}
		),
		items_.end()
	);
	chests_.erase(
		std::remove_if(chests_.begin(), chests_.end(),
			[](const std::unique_ptr<Chest>& chest) {
				return chest->IsDead();
			}
		),
		chests_.end()
	);

	// 更新
	for (auto& i : items_) {
		i->Update();
		// 自動取得アイテムの移動
		if (i->CanAutoGet() && !isCombat) {
			i->MoveToPlayer(ToXZ(player->GetTransform().translate));
		}
	}
	for (auto& c : chests_) {
		c->Update();

		if (c->CheckDrop()) {
			// アイテム出現
			ChestItemDrop(c.get());
		}
	}

	// 敵全滅時にチェスト出現
	if (isCombat_ && !isCombat) {
		std::unique_ptr<Chest> chest = std::make_unique<Chest>(ToXZ(player->GetTransform().translate));
		chests_.push_back(std::move(chest));
	}
	isCombat_ = isCombat;

	// 最短距離にあるアイテムを探す
	interactive_ = nullptr;
	float closestDistance = FLT_MAX;
	for (auto& item : items_) {
		float distance = Length(item->GetPosition() - ToXZ(player->GetTransform().translate));
		if (distance < kInteractRange) {
			if (item->CanInteract() && distance < closestDistance) {
				// インタラクト可能アイテムをセット
				closestDistance = distance;
				interactive_ = item.get();

			} else if (item->CanAutoGet()) {
				// 自動取得する
				item->OnPickup(player);
			}
		}
	}
}

void ItemManager::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	auto& render = ctx.Render();

	for (const auto& i : items_) {
		i->Draw();
	}
	for (auto& c : chests_) {
		c->Draw();
	}
	
	// インタラクト可能状態の表示
	if (interactive_) {
		// アイテム説明
		if (WorldPassive* passive = dynamic_cast<WorldPassive*>(interactive_)) {
			passive->DrawDescription();
		}

		// 操作表示
		if (!ctx.Scene().GetCurrentScene()->IsEditMode()) {
			if (input.gamepad.IsConnected()) {
				if (input.gamepad.IsPress(XINPUT_GAMEPAD_A)) {
					controlPad_->SetTextureRect(64 * 3, 64 * 8, 64, 64);
				} else {
					controlPad_->SetTextureRect(64 * 2, 64 * 8, 64, 64);
				}
				render.DrawSprite(controlPad_.get());
			} else {
				if (input.keyboard.IsPress(DIK_F)) {
					controlKey_->SetTextureRect(64 * 0, 64 * 9, 64, 64);
				} else {
					controlKey_->SetTextureRect(64 * 1, 64 * 9, 64, 64);
				}
				render.DrawSprite(controlKey_.get());
			}
		}
	}
}

void ItemManager::Interact(Player* player) {
	// アイテム毎の取得処理
	if(interactive_) interactive_->OnPickup(player);
}

void ItemManager::SpawnWeapon(Vector3 pos, int index, Rarity rarity, bool isForSale) {
	// 設置する
	auto weapon = std::move(weaponManager_->GetWeapon(index, rarity));
	auto newItem = std::make_unique<WorldWeapon>(std::move(weapon), pos, rarity, isForSale);
	items_.push_back(std::move(newItem));
}

void ItemManager::SpawnMoney(Vector3 pos, int amount) {
	pos.y = 0.5f;
	auto money = std::make_unique<WorldMoney>(pos, amount);
	items_.push_back(std::move(money));
}

void ItemManager::SpawnPassive(Vector3 pos, bool isForSale) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	std::unique_ptr<Passive> p;
	PassiveContext passiveCtx = { enemyManager_, bulletManager_ };

	// 全て出現済みならスキップ
	if (usedPassiveNumbers_.size() > int(PASSIVEITEM::Count)) return;

	// 未使用の番号が出るまで抽選
	PASSIVEITEM r;
	do {
		r = static_cast<PASSIVEITEM>(ctx.RandomInt(0, int(PASSIVEITEM::Count) - 1));
	} while (usedPassiveNumbers_.contains(static_cast<int>(r)));
	// 未使用の番号だけを登録
	usedPassiveNumbers_.insert(static_cast<int>(r));

	switch (r) {
	case PASSIVEITEM::Counter:
		p = std::make_unique<Counter>(
			asset.LoadSprite("Resources/Images/Items/16.png"),
			asset.LoadSprite("Resources/Images/Items/Explain/item_counterAttack.png"),
			passiveCtx);
		break;
	case PASSIVEITEM::Lightning:
		p = std::make_unique<Lightning>(
			asset.LoadSprite("Resources/Images/Items/13.png"),
			asset.LoadSprite("Resources/Images/Items/Explain/item_chainDamage.png"),
			passiveCtx);
		break;
	case PASSIVEITEM::Reload:
		p = std::make_unique<ReloadBoost>(
			asset.LoadSprite("Resources/Images/Items/21.png"),
			asset.LoadSprite("Resources/Images/Items/Explain/item_reload.png"),
			passiveCtx);
		break;
	case PASSIVEITEM::Heal:
		p = std::make_unique<ExtraAttack>(
			asset.LoadSprite("Resources/Images/Items/6.png"),
			asset.LoadSprite("Resources/Images/Items/Explain/item_extraAttack.png"),
			passiveCtx);
		break;
	default:
		p = std::make_unique<Heal>(
			asset.LoadSprite("Resources/Images/Items/14.png"),
			asset.LoadSprite("Resources/Images/Items/Explain/item_lifeSteal.png"),
			passiveCtx);
		break;
	}
	auto newItem = std::make_unique<WorldPassive>(std::move(p), pos, Rarity::Rare, isForSale);
	items_.push_back(std::move(newItem));

	// 拾った場合に出せなくする
	pickedPassiveNumbers_ = usedPassiveNumbers_;
}

void ItemManager::SpawnChest(Vector3 pos) {
}

void ItemManager::Drop(Vector3 pos, std::unique_ptr<Weapon> weapon) {
	// 武器を落とす
	if (weapon) {
		pos.y = 0.5f;
		auto newItem = std::make_unique<WorldWeapon>(std::move(weapon), pos, weapon->GetData().rarity);
		items_.push_back(std::move(newItem));
	}
}

void ItemManager::SetSpawn(Vector3 pos, int index) {
	nextSpawnPos_ = pos;
	nextSpawnIndex_ = index;
}

void ItemManager::Reset() {
	items_.clear();
	weaponSpawned_.clear();
	weaponSaleSpawned_.clear();
	passiveSpawned_.clear();
	passiveSaleSpawned_.clear();

	// 出現して取得していないパッシブを再出現可能に
	usedPassiveNumbers_ = pickedPassiveNumbers_;
}

void ItemManager::Load() {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();
	Reset();

	std::vector<InstancedModel*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<InstancedModel*>(obj)) {
			auto* model = dynamic_cast<InstancedModel*>(obj);

			if (model->tag == "weaponSpawn") {
				int size = int(model->GetTransforms().size());
				for (int i = 0; i < size; ++i) {
					Transform t = model->GetTransforms()[i];
					if (t.scale == Vector3{ 0,0,0 } || t.translate == Vector3{ 0,0,0 }) continue;
					if (int(weaponSpawned_.size()) < i + 1) weaponSpawned_.resize(i + 1);

					Vector3 pos = Vector3{ t.translate.x, 0.5f, t.translate.z };
					if (!weaponSpawned_[i]) {
						SpawnWeapon(pos, -1, Common, false);
						weaponSpawned_[i] = true;
					}
				}
			}

			if (model->tag == "weaponForSale") {
				int size = int(model->GetTransforms().size());

				for (int i = 0; i < size; ++i) {
					Transform t = model->GetTransforms()[i];
					if (t.scale == Vector3{ 0,0,0 } || t.translate == Vector3{ 0,0,0 }) continue;
					if (int(weaponSaleSpawned_.size()) < i + 1) weaponSaleSpawned_.resize(i + 1);
					Vector3 pos = Vector3{ t.translate.x, 0.5f, t.translate.z };
					if (!weaponSaleSpawned_[i]) {
						SpawnWeapon(pos, -1, Common, true);
						weaponSaleSpawned_[i] = true;
					}
				}
			}

			if (model->tag == "passive") {
				int size = int(model->GetTransforms().size());

				for (int i = 0; i < size; ++i) {
					Transform t = model->GetTransforms()[i];
					if (t.scale == Vector3{ 0,0,0 } || t.translate == Vector3{ 0,0,0 }) continue;
					if (int(passiveSpawned_.size()) < i + 1) passiveSpawned_.resize(i + 1);
					Vector3 pos = Vector3{ t.translate.x, 0.5f, t.translate.z };
					if (!passiveSpawned_[i]) {
						SpawnPassive(pos, false);
						passiveSpawned_[i] = true;
					}
				}
			}

			if (model->tag == "passiveForSale") {
				int size = int(model->GetTransforms().size());

				for (int i = 0; i < size; ++i) {
					Transform t = model->GetTransforms()[i];
					if (t.scale == Vector3{ 0,0,0 } || t.translate == Vector3{ 0,0,0 }) continue;
					if (int(passiveSaleSpawned_.size()) < i + 1) passiveSaleSpawned_.resize(i + 1);
					Vector3 pos = Vector3{ t.translate.x, 0.5f, t.translate.z };
					if (!passiveSaleSpawned_[i]) {
						SpawnPassive(pos, true);
						passiveSaleSpawned_[i] = true;
					}
				}
			}
		}
	}
}

void ItemManager::ChestItemDrop(Chest* chest) {
	Vector3 pos = { chest->GetPosition().x, 0, chest->GetPosition().y };

	// ランダム出現
	auto& ctx = GameContext::GetInstance();
	switch (ctx.RandomInt(0, 2)) {
	case 0:
		SpawnMoney(pos, 30);
		break;
	case 1:
		SpawnPassive(pos, false);
		break;
	case 2:
		SpawnWeapon(pos, -1, Common, false);
		break;
	}
}
