#pragma once
#include "Engine/Scene/DebugCamera/DebugCamera.h"
#include "Engine/Graphics/GPUData/LightsForGPU.h"
#include "Engine/SceneObject/LightManager/LightManager.h"
#include "Engine/Graphics/GPUData/CameraForGPU.h"
#include "Engine/SceneObject/Model/Node.h"
#include "Engine/SceneObject/Model/Mesh.h"

#include "Engine/Graphics/Renderer/PostEffectType.h"
#include "Engine/Graphics/GPUData/TransformationMatrix.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"

#include <wrl.h>
#include <d3d12.h>

class Model;
class InstancedModel;
class Sprite;
class Texture;
struct MaterialData;
class ParticleSystem;
class ConstantBufferManager;
class Camera;

class Renderer {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXContext* dxContext);

	/// <summary>
	/// トランスフォーム更新
	/// </summary>
	void UpdateModelTransforms(Model* model);

	void UpdateSpriteTransform(Sprite* sprite);

	/// <summary>
	/// モデル描画
	/// </summary>
	/// <param name="blendMode">ブレンドモード</param>
	void DrawModel(Model* model, LightManager* lightManager, int blendMode);

	/// <summary>
	/// インスタンスモデル描画
	/// </summary>
	/// <param name="model">複数インスタンスを持つモデル</param>
	/// <param name="blendMode">ブレンドモード</param>
	void DrawModelInstance(InstancedModel* model, LightManager* lightManager, int blendMode);

	void DrawParticles(ParticleSystem* particleSys, int blendMode);

	void DrawSprite(Sprite* sprite, int blendMode);

	// ノードごとに描画
	void DrawNode(Model* model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parent);

	// メッシュを描画
	void DrawMesh(Model* model, Mesh* mesh);

	// インスタンシング描画版
	void DrawNodeInstance(InstancedModel* model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld);
	void DrawMeshInstance(InstancedModel* model, Mesh* mesh);

	// Skybox描画
	void DrawSkybox(Texture* texture);

	// ポストエフェクト設定
	void SetPostEffectType(PostEffectType type);

	// カメラセット
	void SetCamera(Camera* camera);
	Camera* GetCamera() { return camera_; }

	/// <summary>
	/// フレーム開始時の処理(描画開始時に行う)
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// フレーム終了時の処理(描画終了時に行う)
	/// </summary>
	void EndFrame();

	// ImGuiシーンのサイズ
	Vector2 GetSceneWindowSize() { return dxContext_->GetSceneWindowSize(); }
private:
	void InitializePlane();
	void InitializeRing();
	void InitializeLine();
	void InitializeSphere();
	void InitializeSkybox();

	DirectXContext* dxContext_ = nullptr;

	// ConstantBuffer管理
	ConstantBufferManager* cbManager_ = nullptr;

	// カメラ位置(GPU転送)
	CameraForGPU* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraBuffer_;

	LightsForGPU* dummyLight_;
	Microsoft::WRL::ComPtr<ID3D12Resource> dummyLightBuffer_;

	struct ShapeData {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vbv;

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
		D3D12_INDEX_BUFFER_VIEW ibv;
	};
	ShapeData plane_;
	ShapeData ring_;
	ShapeData line_;
	ShapeData sphere_;

	// Skybox用データ
	struct SkyboxData {
		ShapeData shapeData;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> transformResource;

		MaterialData* materialData = nullptr;
		TransformationMatrix* transformData = nullptr;
	};
	SkyboxData skybox;

	Camera* camera_;
};