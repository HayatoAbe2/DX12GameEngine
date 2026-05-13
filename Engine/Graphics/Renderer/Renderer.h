#pragma once
#include "Engine/Scene/DebugCamera/DebugCamera.h"
#include "Engine/Graphics/GPUData/LightsForGPU.h"
#include "Engine/Object/LightManager/LightManager.h"
#include "Engine/Graphics/GPUData/CameraForGPU.h"
#include "Engine/Asset/Model/Node.h"
#include "Engine/Asset/Model/Mesh.h"

#include "Engine/Graphics/GPUData/TransformationMatrix.h"

#include <wrl.h>
#include <d3d12.h>

class Model;
class InstancedModel;
class Sprite;
class Texture;
class Material;
class ParticleSystem;
class DirectXContext;
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
	void UpdateModelTransforms(Model* model, Camera* camera);

	void UpdateSpriteTransform(Sprite* sprite);

	/// <summary>
	/// モデル描画
	/// </summary>
	/// <param name="blendMode">ブレンドモード</param>
	void DrawModel(Model* model, Camera* camera, LightManager* lightManager, int blendMode);

	/// <summary>
	/// インスタンスモデル描画
	/// </summary>
	/// <param name="model">複数インスタンスを持つモデル</param>
	/// <param name="blendMode">ブレンドモード</param>
	void DrawModelInstance(InstancedModel* model, Camera* camera, LightManager* lightManager, int blendMode);

	void DrawParticles(ParticleSystem* particleSys, Camera* camera, int blendMode);

	void DrawSprite(Sprite* sprite, int blendMode);

	// ノードごとに描画
	void DrawNode(Model* model, Camera* camera, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parent);

	// メッシュを描画
	void DrawMesh(Model* model, Mesh* mesh);

	// インスタンシング描画版
	void DrawNodeInstance(InstancedModel* model, Camera* camera, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld);
	void DrawMeshInstance(InstancedModel* model, Mesh* mesh);

	// Skybox描画
	void DrawSkybox(Texture* texture, Camera* camera);

	/// <summary>
	/// フレーム開始時の処理(描画開始時に行う)
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// フレーム終了時の処理(描画終了時に行う)
	/// </summary>
	void EndFrame();

private:
	void InitializeAABB(AABB aabb);
	void InitializeSkybox();

	DirectXContext* dxContext_ = nullptr;

	// ConstantBuffer管理
	ConstantBufferManager* cbManager_ = nullptr;

	// カメラ位置(GPU転送)
	CameraForGPU* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraBuffer_;

	LightsForGPU* dummyLight_;
	Microsoft::WRL::ComPtr<ID3D12Resource> dummyLightBuffer_;

	// Skybox用データ
	struct SkyboxData {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vbv;

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
		D3D12_INDEX_BUFFER_VIEW ibv;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		Microsoft::WRL::ComPtr<ID3D12Resource> transformResource;

		Material* materialData = nullptr;
		TransformationMatrix* transformData = nullptr;
	};
	SkyboxData skybox;
};