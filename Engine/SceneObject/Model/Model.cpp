#include "Model.h"
#include "SubMesh.h"

void Model::Update() {
	// アニメーション更新
	if (!animationPlayer_) {
		return;
	}

	if (skeleton_.joints.size() > 1) {
		// ボーンあり
		animationPlayer_->Update(skeleton_);
		UpdateSkeleton();
		UpdateSkinCluster();
	} else if (animationPlayer_) {
		animationPlayer_->Update(*rootNode_.get());
	}
}

void Model::UpdateSkeleton() {
	// スケルトン内ジョイントの行列更新
	for (Joint& joint : skeleton_.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		// 親がいる場合親の行列を掛ける
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton_.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Model::UpdateSkinCluster() {
	for (size_t jointIndex = 0; jointIndex < skeleton_.joints.size(); ++jointIndex) {
		assert(jointIndex < skeleton_.joints.size());

		for (auto& mesh : meshes_) {
			for (auto& primitive : mesh.subMeshes) {
				skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix = data_->skinClusterData_.inverseBindPoseMatrices[jointIndex] * skeleton_.joints[jointIndex].skeletonSpaceMatrix;
				skinCluster_.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix = Transpose(Inverse(skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix));
			}
		}
	}
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
		materials_.push_back(std::move(newMat));
	}
}

void Model::ChangeAnimation(std::shared_ptr<Animation> animation, float time) {
	if (animationPlayer_) {
		animationPlayer_->ChangeAnimation(animation, time);
	}
}