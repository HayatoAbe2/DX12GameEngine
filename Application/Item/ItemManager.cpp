#include "ItemManager.h"
#include "Character/Player/Player.h"
#include "Weapon/WeaponManager.h"
#include "Weapon/Weapon.h"
#include <fstream>
#include <sstream>

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
}

void ItemManager::Update(Player* player) {
	items_.erase(
		std::remove_if(items_.begin(), items_.end(),
			[](const std::unique_ptr<Item>& item) {
				return item->IsDead();
			}
		),
		items_.end()
	);

	// 最短のアイテムを探す
	int closestIndex = -1;
	float closestDistance = FLT_MAX;
	for (auto& item : items_) {
		float distance = Length(item->GetTransform().translate - player->GetTransform().translate);
		if (distance < closestDistance && distance < 2.0f) {
			closestDistance = distance;
			closestIndex = int(&item - &items_[0]);
		}
	}

	// 取得範囲内
	if (player->GetInteractRadius() >= closestDistance) {
		canInteract_ = true;
	} else {
		canInteract_ = false;
	}
}

void ItemManager::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& input = ctx.Input();
	auto& render = ctx.Render();

	for (const auto& item : items_) {
		item->Draw(camera);
	}

	if (canInteract_) {
		if (input.gamepad.IsConnected()) {
			if (input.gamepad.IsPress(XINPUT_GAMEPAD_A)) {
				controlPad_->SetTextureRect(64 * 2, 64 * 8, 64, 64);
			} else {
				controlPad_->SetTextureRect(64 * 3, 64 * 8, 64, 64);
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
		float distance = Length(item->GetTransform().translate - player->GetTransform().translate);
		if (distance < closestDistance && distance < 2.0f) {
			closestDistance = distance;
			closestIndex = int(&item - &items_[0]);
		}
	}

	// 取得範囲内なら取る
	if (player->GetInteractRadius() >= closestDistance) {
		Drop(player->GetTransform().translate, player->DropWeapon());
		player->SetWeapon(std::move(items_[closestIndex]->GetWeapon()));
		items_[closestIndex]->Erase();
	}
}

void ItemManager::Spawn(Vector3 pos, int index) {
	auto& ctx = GameContext::GetInstance();

	// レア度ランダム
	Rarity rarity = Common;
	Spawn(pos, index, rarity);
}

void ItemManager::Spawn(Vector3 pos, int index, Rarity rarity) {
	// 設置する
	auto weapon = std::move(weaponManager_->GetWeapon(index, rarity));
	auto newItem = std::make_unique<Item>(std::move(weapon), pos, rarity);
	items_.push_back(std::move(newItem));
}

void ItemManager::Drop(Vector3 pos, std::unique_ptr<Weapon> weapon) {
	// アイテムを落とす
	if (weapon) {
		auto newItem = std::make_unique<Item>(std::move(weapon), pos + Vector3{0,0.5f,0}, weapon->GetData().rarity);
		items_.push_back(std::move(newItem));
	}
}

void ItemManager::Reset() {
	items_.clear();
}

void ItemManager::LoadCSV(const std::string& filePath, const float tileSize) {
	std::ifstream file(filePath);
	std::string line;

	assert(file.is_open());

	std::getline(file, line); // 最初の行をスキップ

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string itemStr, xStr, zStr;

		std::getline(ss, itemStr, ',');
		std::getline(ss, xStr, ',');
		std::getline(ss, zStr, ',');

		int itemNum = std::stoi(itemStr);
		float x = std::stof(xStr);
		float z = std::stof(zStr);

		Vector3 pos = Vector3{ x * tileSize, 0.5f, z * tileSize };
		Spawn(pos, itemNum);
	}
}
