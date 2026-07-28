#include "WorldMoney.h"

WorldMoney::WorldMoney(Vector3 pos, int amount) :
	WorldItem(pos, Rarity::None) {

	money_ = amount;
	canInteract_ = false;
	canAutoGet_ = true;

	// コインのモデルはここで生成
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	model_ = asset.LoadModel("Resources/Items", "Coin.obj");
	model_->SetTranslate(pos);
	for (auto& mat : model_->GetMaterials()) {
		auto data = mat->GetData();
		data.color = { 0.8f,0.8f,0,1 };
		mat->SetData(data);
	}
}

void WorldMoney::Update() {
	model_->SetTranslate({ position_.x, 0.5f, position_.y });
	model_->SetRotate(model_->GetTransform().rotate + Vector3(0, 0.05f, 0));
}

void WorldMoney::Draw() {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawModel(model_.get());
}

void WorldMoney::OnPickup(Player* player) {
	// 所持金追加
	player->GetWallet().Add(money_);

	// ワールド上からはなくなる
	isDead_ = true;

}
