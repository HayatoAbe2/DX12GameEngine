#include "ItemManager.h"
#include "Character/Player/Player.h"
#include "Weapon/WeaponManager.h"
#include "Weapon/Weapon.h"
#include "Engine/Scene/BaseScene/BaseScene.h"
#include "Engine/SceneObject/SceneObject.h"

void ItemManager::Initialize(WeaponManager* weaponManager) {
	weaponManager_ = weaponManager;

	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	// 操作
	controlKey_ = asset.LoadSprite("Resources/Control/KeyboardAndMouse.png");
	controlKey_->SetSize({ 64,64 });
	controlKey_->SetPosition({ 640 - 32,720 - 460 });
	controlKey_->SetTextureRect(0, 64 * 9, 64, 64);
	controlPad_ = asset.LoadSprite("Resources/Control/XboxController.png");
	controlPad_->SetSize({ 64,64 });
	controlPad_->SetPosition({ 640 - 32,720 - 460 });
	controlPad_->SetTextureRect(64 * 2, 64 * 8, 64, 64);

	SpawnMoney({ 5,0.5f,5 }, 3);
}

void ItemManager::Update(Player* player) {
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

	// 最短距離にあるアイテムを探す
	int closestIndex = -1;
	float closestDistance = FLT_MAX;
	for (auto& item : items_) {
		float distance = Length(item->GetPosition() - ToXZ(player->GetTransform().translate));
		if (distance < closestDistance && distance < 2.0f) {
			closestDistance = distance;
			closestIndex = int(&item - &items_[0]);
		}
	}

	// 取得範囲内
	if (player->GetInteractRadius() >= closestDistance) {
		// 自動取得
		if (items_[closestIndex]->CanAutoGet()) {
			// アイテム毎の取得処理
			items_[closestIndex]->OnPickup(player);
		}
		
		// インタラクト可能
		if (items_[closestIndex]->CanInteract()) {
			canInteract_ = true;
		}
	} else {
		canInteract_ = false;
	}
}

void ItemManager::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	auto& render = ctx.Render();

	for (const auto& i : items_) {
		i->Draw();
	}


	if (canInteract_ && !ctx.Scene().GetCurrentScene()->IsEditMode()) {
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

void ItemManager::Interact(Player* player) {
	// 最短のアイテムを探す
	int closestIndex = -1;
	float closestDistance = FLT_MAX;
	for (auto& item : items_) {
		if (!item->CanInteract()) { continue; }

		float distance = Length(item->GetPosition() - ToXZ(player->GetTransform().translate));
		if (distance < closestDistance && distance < 2.0f) {
			closestDistance = distance;
			closestIndex = int(&item - &items_[0]);
		}
	}

	// 取得範囲内なら取る
	if (player->GetInteractRadius() >= closestDistance) {
		// アイテム毎の取得処理
		items_[closestIndex]->OnPickup(player);
	}
}

void ItemManager::SpawnWeapon(Vector3 pos, int index, Rarity rarity) {
	// 設置する
	auto weapon = std::move(weaponManager_->GetWeapon(index, rarity));
	auto newItem = std::make_unique<WorldWeapon>(std::move(weapon), pos, rarity);
	items_.push_back(std::move(newItem));
}

void ItemManager::SpawnMoney(Vector3 pos, int amount) {
	auto money = std::make_unique<WorldMoney>(pos, amount);
	items_.push_back(std::move(money));
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
	//weapons_.clear();
	spawned_.clear();
}

void ItemManager::Load() {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	std::vector<InstancedModel*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<InstancedModel*>(obj)) {
			auto* model = dynamic_cast<InstancedModel*>(obj);
			models.push_back(model);
		}
	}

	for (auto& model : models) {
		if (model->tag == "weaponSpawn") {
			int size = int(model->GetTransforms().size());
			if (int(spawned_.size()) < size) spawned_.resize(size);

			for (int i = 0; i < model->GetTransforms().size(); ++i) {
				Transform t = model->GetTransforms()[i];
				if (t.scale == Vector3{ 0,0,0 }) continue;
				
				Vector3 pos = Vector3{ t.translate.x, 0.5f, t.translate.z };
				if (!spawned_[i]) {
					SpawnWeapon(pos, -1, Common);
					spawned_[i] = true;
				}
			}
		}
	}
}
