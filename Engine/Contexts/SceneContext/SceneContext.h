#pragma once
#include <string>
#include <vector>
#include "Engine/Math/Vector3/Vector3.h"

class BaseScene;
class SceneManager;
class SceneEditor;
class Model;
class InstancedModel;
class SceneContext {
public:
	SceneContext(SceneManager* sceneManager, SceneEditor* sceneEditor);

	// シーン変更
	void SceneChange(std::string nextSceneName);

	BaseScene* GetCurrentScene();
	void SceneLoad(const std::string& path, Vector3 offset = Vector3{0,0,0});
	void Reset();

	// タグで検索
	std::vector<Model*> FindModelsByTag(const std::string& tag);
	std::vector<InstancedModel*> FindInstancedModelsByTag(const std::string& tag);
private:
	SceneManager* sceneManager_ = nullptr;
	SceneEditor* sceneEditor_ = nullptr;
};

