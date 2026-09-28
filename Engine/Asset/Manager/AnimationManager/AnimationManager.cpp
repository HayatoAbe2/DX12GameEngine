#include "AnimationManager.h"
#include <cassert>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::vector<std::shared_ptr<Animation>> AnimationManager::Load(const std::string& directoryPath, const std::string& filename, const std::function<uint32_t()>& generateID) {
	std::string filePath = directoryPath + "/" + filename;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0); // アニメーションなし

	std::vector<std::shared_ptr<Animation>> animations;
	for (uint32_t animationIndex = 0;
		animationIndex < scene->mNumAnimations;
		++animationIndex) {
		std::shared_ptr<Animation> animation = std::make_shared<Animation>(generateID());
		aiAnimation* animationAssimp = scene->mAnimations[animationIndex];

		// 名前
		animation->name_ = animationAssimp->mName.C_Str();

		// 秒数
		animation->duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

		// channelからNodeAnimationの情報を取得
		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
			aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = animation->nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

			// キーフレーム
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
				// Translate
				aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyframeVector3 keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // 秒に変換
				keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 左手座標系に変換
				nodeAnimation.translate.push_back(keyframe);
			}

			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
				// Rotate
				aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyframeQuaternion keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
				nodeAnimation.rotate.push_back(keyframe);
			}

			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
				// Scale
				aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyframeVector3 keyframe;
				keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
				nodeAnimation.scale.push_back(keyframe);
			}
		}
		animations.push_back(animation);
	}

	return animations;
}