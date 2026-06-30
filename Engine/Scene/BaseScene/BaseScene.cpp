#include "BaseScene.h"

void BaseScene::Draw() {
	auto& render = GameContext::GetInstance().Render();

	for (auto& object : sceneObjects_) {
		if(auto* model = dynamic_cast<Model*>(object.get())){
			render.DrawModel(model);
		}else if (auto* model = dynamic_cast<InstancedModel*>(object.get())) {
			render.DrawInstancedModel(model);
		}else if (auto* sprite = dynamic_cast<Sprite*>(object.get())) {
			render.DrawSprite(sprite);
		}
	}
}
