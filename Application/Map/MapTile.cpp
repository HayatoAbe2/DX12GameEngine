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

void MapTile::Initialize(std::unique_ptr<InstancedModel> wall, std::unique_ptr<InstancedModel> floor, std::unique_ptr<Model> goal) {
	auto& ctx = GameContext::GetInstance();
	auto& asset = ctx.Asset();

	wall_ = std::move(wall);
	floor_ = std::move(floor);
	goal_ = std::move(goal);
	
	auto matData = wall_->GetMaterial(0)->GetData();
	matData.color = { 0.3f,0.3f,0.3f,1 };
	wall_->GetMaterial(0)->SetData(matData);

	matData = floor_->GetMaterial(0)->GetData();
	matData.color = { 0.5f,0.5f,0.5f,1 };
	wall_->GetMaterial(0)->SetData(matData);


	particle_ = std::make_unique<ParticleSystem>();
	particle_->Initialize(asset.LoadInstancedModel("Resources/Particle/Goal", "goalEffect.obj", particleNum_));
	particle_->SetLifeTime(40);
	particle_->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });
}

void MapTile::LoadCSV(const std::string& filePath) {
	auto& ctx = GameContext::GetInstance();
	auto& light = ctx.Light();

	soundPlayed_ = false;

	if (lightIndex_ != -1) { light.RemoveSpotLight(lightIndex_); }
	particle_->RemoveField();
	map_.clear();
	mapWidth_ = 0;
	mapHeight_ = 0;

	std::ifstream file(filePath);
	std::string line;

	if (!file.is_open()) {
		assert(false);
	}
	assert(file.is_open());

	// ファイル読む
	while (std::getline(file, line)) {
		std::vector<Tile> row;
		std::stringstream ss(line);
		std::string cell;

		while (std::getline(ss, cell, ',')) {
			int value = std::stoi(cell);
			row.push_back(Tile(value));
		}

		if (mapWidth_ == 0) { // 一度だけ
			mapWidth_ = static_cast<int>(row.size());
		}
		map_.push_back(row);
	}
	mapHeight_ = static_cast<int>(map_.size());
	std::reverse(map_.begin(), map_.end());

	// トランスフォーム設定
	std::vector<Transform> transformsFloor;
	std::vector<Transform> transformsWall;
	Transform transformGoal;
	for (int x = 0; x < mapWidth_; ++x) {
		for (int y = 0; y < mapHeight_; ++y) {
			// 床
			Transform transformFloor;
			if (map_[y][x] == Tile::Floor) {
				transformFloor.translate.x = float(x) * tileSize_ + tileSize_ / 2.0f;
				transformFloor.translate.y = -tileSize_;
				transformFloor.translate.z = float(y) * tileSize_ + tileSize_ / 2.0f;
				transformFloor.scale = { tileSize_,tileSize_ ,tileSize_ };
			} else {
				transformFloor.translate.y = 200.0f;
			}
			transformsFloor.push_back(transformFloor);

			// 壁
			Transform transformWall;
			if (map_[y][x] == Tile::LeftWall ||
				map_[y][x] == Tile::RightWall ||
				map_[y][x] == Tile::UpWall ||
				map_[y][x] == Tile::BottomWall) {
				transformWall.translate.x = float(x) * tileSize_ + tileSize_ / 2.0f;
				transformWall.translate.y = 0;
				transformWall.translate.z = float(y) * tileSize_ + tileSize_ / 2.0f;
				transformWall.scale = { tileSize_,tileSize_ ,tileSize_ };

				// 方向に応じて回転

			} else {
				transformWall.translate.y = 200.0f;
			}
			transformsWall.push_back(transformWall);


			// ゴール
			if (map_[y][x] == Tile::Goal) {
				transformGoal.translate.x = float(x) * tileSize_ + tileSize_ / 2.0f;
				transformGoal.translate.y = 0;
				transformGoal.translate.z = float(y) * tileSize_ + tileSize_ / 2.0f;
				transformGoal.scale = { tileSize_,tileSize_ ,tileSize_ };

				lightIndex_ = light.AddSpotLight();
				auto& spotLight = light.GetSpotLight(lightIndex_);
				spotLight.position = transformGoal.translate + Vector3{ 0.0f,3.0f,0.0f };
				spotLight.direction = { 0,-1.0f,0 };
				spotLight.intensity = 1.0f;
				spotLight.color = { 1,1,0,0.3f };
				spotLight.distance = 10.0f;
				spotLight.decay = 0.9f;
				spotLight.cosAngle = 0.32f;
				spotLight.cosFalloffStart = 1.1f;

				std::unique_ptr<ParticleField> particleField = std::make_unique<ParticleField>();
				particleField->SetCheckArea(false);
				particleField->SetRotateXZ(0.15f, { x * tileSize_ + tileSize_ / 2.0f,0,y * tileSize_ + tileSize_ / 2.0f });
				particle_->AddField(std::move(particleField));
			}
		}
	}
	int preSize = int(transformsWall.size());
	transformsWall.resize(wall_->GetNumInstance());
	for (int i = preSize; i < int(wall_->GetTransforms().size()); ++i) {
		transformsWall[i].scale = {};
	}

	preSize = int(transformsFloor.size());
	transformsFloor.resize(floor_->GetNumInstance());
	for (int i = preSize; i < int(floor_->GetTransforms().size()); ++i) {
		transformsFloor[i].scale = {};
	}

	floor_->SetInstanceTransforms(transformsFloor);
	wall_->SetInstanceTransforms(transformsWall);
	goal_->SetTransform(transformGoal);
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
			Vector3 randomVector = Normalize({
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f),
			0.0f,
			ctx.RandomFloat(-particleRange_ / 2.0f, particleRange_ / 2.0f)
				}) * particleRange_ / 2.0f;

			Transform transform = goal_->GetTransform();
			transform.translate += randomVector;
			transform.scale = { 0.5f,4.0f,4.0f };
			particle_->Emit(transform, { 0,0.5f,0 });

			emitTimer_ = 0;
		}

		MaterialData materialData = goal_->GetData()->defaultMaterials_[0]->GetData();
		materialData.color = { 1,1,1,1 };
		goal_->GetData()->defaultMaterials_[0]->SetData(materialData);

		light.GetSpotLight(lightIndex_).intensity = 1.0f;
	} else {
		MaterialData materialData = goal_->GetData()->defaultMaterials_[0]->GetData();
		materialData.color = { 0.1f,0.1f,0.1f,1.0f };
		goal_->GetData()->defaultMaterials_[0]->SetData(materialData);

		light.GetSpotLight(lightIndex_).intensity = 0.0f;
	}

	particle_->Update();
}

void MapTile::Draw(Camera* camera) {
	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();

	render.DrawInstancedModel(wall_.get(), camera);
	render.DrawInstancedModel(floor_.get(), camera);
	render.DrawModel(goal_.get(), camera);
	render.DrawInstancedModel(particle_->GetInstancedModel(), camera, BlendMode::Add);
}
