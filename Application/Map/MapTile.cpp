#include "MapTile.h"
#include "GameCommon.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <memory>

MapTile::~MapTile() {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	light.RemoveSpotLight(lightIndex_);
}

void MapTile::Initialize() {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();
	auto& light = ctx.Light();

	particle_ = asset.CreateParticleSystem(asset.CreateMaterial(asset.LoadTexture("Resources/Particle/Goal/circle.png")), particleNum_);
	particle_->SetLifeTime(40);
	particle_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });

	lightIndex_ = light.AddSpotLight();
}

void MapTile::UpdateMapChange(bool isCombat, bool isEditMode) {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();
	auto& scene = ctx.Scene();

	std::vector<InstancedModel*> models;
	for (auto& obj : scene.GetCurrentScene()->GetObjects()) {
		if (dynamic_cast<InstancedModel*>(obj)) {
			auto* model = dynamic_cast<InstancedModel*>(obj);
			models.push_back(model);
		}
	}

	map_.clear();
	map_.assign(mapHeight_, std::vector<Tile>(mapWidth_, Tile::None));

	for (auto& model : models) {
		auto matData = model->GetMaterial(0)->GetData();

		if (model->tag == "floor") {
			for (Transform& t : model->GetTransforms()) {
				int tileX = static_cast<int>(std::floor(t.translate.x / tileSize_ + 0.5f));
				int tileY = static_cast<int>(std::floor(t.translate.z / tileSize_ + 0.5f));
				int x = tileX + mapWidth_ / 2;
				int y = tileY + mapHeight_ / 2;

				if (x >= 0 && x < mapWidth_ &&
					y >= 0 && y < mapHeight_) {
					map_[y][x] = Tile::Floor;
				}
			}
		}
		if (model->tag == "wall") {
			for (Transform& t : model->GetTransforms()) {
				if (t.scale == Vector3{ 0,0,0 }) continue;
				int tileX = static_cast<int>(std::floor(t.translate.x / tileSize_ + 0.5f));
				int tileY = static_cast<int>(std::floor(t.translate.z / tileSize_ + 0.5f));
				int x = tileX + mapWidth_ / 2;
				int y = tileY + mapHeight_ / 2;

				if (x >= 0 && x < mapWidth_ &&
					y >= 0 && y < mapHeight_) {
					map_[y][x] = Tile::Wall;
				}
			}
		}
		if (model->tag == "barrier") {
			for (Transform& t : model->GetTransforms()) {
				int tileX = static_cast<int>(std::floor(t.translate.x / tileSize_ + 0.5f));
				int tileY = static_cast<int>(std::floor(t.translate.z / tileSize_ + 0.5f));
				int x = tileX + mapWidth_ / 2;
				int y = tileY + mapHeight_ / 2;

				if (x >= 0 && x < mapWidth_ &&
					y >= 0 && y < mapHeight_) {
					map_[y][x] = Tile::CombatWall;
				}
			}
			if (isCombat || isEditMode) {
				matData.color = { 0.3f, 0.3f, 1.0f, 0.5f };
			} else {
				matData.color = { 0.3f, 0.3f, 1.0f, 0.0f };
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);

		}
		if (model->tag == "goal") {
			for (Transform& t : model->GetTransforms()) {
				int tileX = static_cast<int>(std::floor(t.translate.x / tileSize_ + 0.5f));
				int tileY = static_cast<int>(std::floor(t.translate.z / tileSize_ + 0.5f));
				int x = tileX + mapWidth_ / 2;
				int y = tileY + mapHeight_ / 2;

				if (x >= 0 && x < mapWidth_ &&
					y >= 0 && y < mapHeight_) {
					map_[y][x] = Tile::Goal;
				}

				auto& spotLight = light.GetSpotLight(lightIndex_);
				spotLight.position = t.translate + Vector3{ 0.0f,3.0f,0.0f };
				spotLight.direction = { 0,-1.0f,0 };
				spotLight.intensity = 1.0f;
				spotLight.color = { 1,1,0,0.3f };
				spotLight.distance = 10.0f;
				spotLight.decay = 0.9f;
				spotLight.cosAngle = 0.32f;
				spotLight.cosFalloffStart = 1.1f;

				std::unique_ptr<ParticleField> particleField = std::make_unique<ParticleField>();
				particleField->SetCheckArea(false);
				particleField->SetRotateXZ(0.15f, { x * tileSize_ + tileSize_ / 2.0f, 0, y * tileSize_ + tileSize_ / 2.0f });
				particle_->RemoveField();
				particle_->AddField(std::move(particleField));
				particleTransform_.translate = Vector3{ x * tileSize_, 0, y * tileSize_ };
			}
		}

		if (model->tag == "weaponSpawn") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 0.3f, 1.0f, 0.3f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		} else if (model->tag == "weaponForSale") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 1.0f, 1.0f, 0.0f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		} else if (model->tag == "passive") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 0.3f, 1.0f, 0.3f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		} else if (model->tag == "passiveForSale") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 1.0f, 1.0f, 0.0f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		} else  if (model->tag == "enemySpawn") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 0.7f, 0.3f, 0.3f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		} else if (model->tag == "enemySpawnPoint") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 1.0f, 0.0f, 0.0f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		} else if (model->tag == "roomConnector") {
			if (scene.GetCurrentScene()->IsEditMode()) {
				matData.color = { 1.0f, 1.0f, 1.0f, 0.5f };
			} else {
				matData.color.w = 0;
			}
			model->GetMaterial(0)->SetData(matData);
			model->GetMaterial(1)->SetData(matData);
		}
	}
}

void MapTile::Update(bool canGoal) {
	auto& ctx = GameContext::GetInstance();
	auto& audio = ctx.Audio();
	auto& light = ctx.Light();
	if (canGoal) {
		if (!soundPlayed_) {
			audio.SoundPlay(L"Resources/Sounds/SE/floorClear.mp3", false);
			soundPlayed_ = true;
		}

		emitTimer_++;
		if (emitTimer_ >= emitTime_) {
			// パーティクル
			Vector3 randomVector = Normalize(Vector3{
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			0.0f,
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f)
				}) * particleRange_ / 2.0f;

			Transform transform = particleTransform_;
			transform.translate += randomVector;
			transform.scale = { 0.5f,4.0f,4.0f };
			particle_->Emit(transform, { 0,0.5f,0 });

			emitTimer_ = 0;
		}
		light.GetSpotLight(lightIndex_).intensity = 1.0f;
	}

	particle_->Update();
}

void MapTile::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawParticle(particle_.get(), BlendMode::Add);
}
