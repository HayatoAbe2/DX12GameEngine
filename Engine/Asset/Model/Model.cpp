#include "Model.h"

void Model::Update() {
	// アニメーション更新
	animationPlayer_->Update(*rootNode_.get());
}

void Model::CopyModelData(std::shared_ptr<ModelData> data, BufferManager* bufferManager) {
	// データ設定
	data_ = data;

	// 現在のマテリアルを共通マテリアルからコピー
	for (int i = 0; i < data->defaultMaterials_.size(); ++i) {
		auto newMat = std::make_unique<Material>(*data->defaultMaterials_[i].get());
		newMat->Initialize(bufferManager, data->defaultMaterials_[i]->GetData().useTexture, data->defaultMaterials_[i]->GetData().enableLighting);
		newMat->SetData(data->defaultMaterials_[i]->GetData());
		newMat->SetTexture(data->defaultMaterials_[i]->GetTexture());
		material_.push_back(std::move(newMat));
	}
}
