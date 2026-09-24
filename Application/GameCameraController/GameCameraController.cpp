#include "GameCameraController.h"

GameCameraController::GameCameraController(Camera* camera) {
	camera_ = camera;

	auto& ctx = GameContext::GetInstance();
	auto& render = ctx.Render();
	render.SetCamera(camera_);

	camera_->transform_.rotate = { 1.0f,0,0 };
}

void GameCameraController::Update(Vector3 center) {
	auto& ctx = GameContext::GetInstance();
	auto& scene = ctx.Scene();

	// ウィンドウの四隅
	Vector2 windowSize = ctx.GetRenderWindowSize();
	ScreenToWorldOnGround({ 0,0 });
	ScreenToWorldOnGround({ windowSize.x, 0 });
	ScreenToWorldOnGround({ 0, windowSize.y });
	ScreenToWorldOnGround({ windowSize.x, windowSize.y });

	// カメラの移動範囲制限
	float minX = -FLT_MAX; float maxX = FLT_MAX;
	float minZ = -FLT_MAX; float maxZ = FLT_MAX;

	std::vector<Model*> west = scene.FindModelsByTag("westConnector");
	std::vector<Model*> east = scene.FindModelsByTag("eastConnector");
	std::vector<Model*> south = scene.FindModelsByTag("southConnector");
	std::vector<Model*> north = scene.FindModelsByTag("northConnector");
	if(!west.empty()) minX = west[0]->GetTransform().translate.x;
	if(!east.empty()) maxX = east[0]->GetTransform().translate.x;
	if(!south.empty()) minZ = south[0]->GetTransform().translate.z;
	if(!north.empty()) maxZ = north[0]->GetTransform().translate.z;

	float offsetW = 12.55f;
	float offsetE = 12.55f;
	float offsetN = 10.0f;
	float offsetS = 6.5f;

	// 範囲内に移動
	center.x = std::clamp(center.x, minX + offsetW, maxX - offsetE);
	center.z = std::clamp(center.z, minZ + offsetS, maxZ - offsetN);

	// 移動
	camera_->transform_.translate = center + Vector3{ 0,30,-19 };
}

Vector3 GameCameraController::ScreenToWorldOnGround(Vector2 screenPos) {
	auto& ctx = GameContext::GetInstance();
	Vector2 window = ctx.GetRenderWindowSize();

	// スクリーン → NDC
	float ndcX = (screenPos.x / window.x) * 2.0f - 1.0f;
	float ndcY = 1.0f - (screenPos.y / window.y) * 2.0f;

	// NDC → Clip
	Vector4 clipNear(ndcX, ndcY, 0.0f, 1.0f);
	Vector4 clipFar(ndcX, ndcY, 1.0f, 1.0f);

	// ViewProjection の逆行列
	Matrix4x4 invVP = Inverse(camera_->viewMatrix_ * camera_->projectionMatrix_);

	Vector4 worldNear = TransformVector(clipNear, invVP);
	Vector4 worldFar = TransformVector(clipFar, invVP);

	worldNear /= worldNear.w;
	worldFar /= worldFar.w;

	// レイ
	Vector3 origin = {
		worldNear.x,
		worldNear.y,
		worldNear.z
	};

	Vector3 end = {
		worldFar.x,
		worldFar.y,
		worldFar.z
	};

	Vector3 direction = end - origin;

	// y = 0 との交点
	float t = -origin.y / direction.y;

	return origin + direction * t;
}
