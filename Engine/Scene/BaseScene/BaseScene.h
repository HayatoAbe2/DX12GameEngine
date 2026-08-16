#pragma once
#include "GameCommon.h"
#include "Engine/SceneObject/SceneObject.h"

/// <summary>
/// 各シーンの基底クラス
/// </summary>
class BaseScene {
public:

	virtual ~BaseScene() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw();

	// 登録オブジェクト取得
	std::vector<SceneObject*> GetObjects() {
		std::vector<SceneObject*> objects;
		for (auto& object : sceneObjects_) {
			objects.push_back(object.get());
		}
		return objects;
	}

	// オブジェクト追加
	void AddObject(std::unique_ptr<SceneObject> object) { sceneObjects_.push_back(std::move(object)); }
	// オブジェクト削除
	void RemoveObject(SceneObject* object) {
		auto it = std::find_if(
			sceneObjects_.begin(),
			sceneObjects_.end(),
			[&](const std::unique_ptr<SceneObject>& ptr) {
				return ptr.get() == object;
			}
		);

		if (it != sceneObjects_.end()) {
			pendingDelete_.push_back(std::move(*it));
			sceneObjects_.erase(it);
		}
	}
	void Clear() {
		for (auto& obj : sceneObjects_) {
			pendingDelete_.push_back(std::move(obj));
		}

		sceneObjects_.clear();
	}

	void FlushDelete() {
		pendingDelete_.clear();
	}

	bool IsEditMode() { return enableEditMode_; }
	void SetEditMode(bool editFlag) { enableEditMode_ = editFlag; }
protected:

	// カメラ
	std::unique_ptr<Camera> camera_ = nullptr;
	float cameraDistance_ = 20.0f;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;

	// オブジェクト
	std::vector<std::unique_ptr<SceneObject>> sceneObjects_;
	std::vector<std::unique_ptr<SceneObject>> pendingDelete_;

	bool enableEditMode_ = false;
};

