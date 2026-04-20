#pragma once
#include "Engine/Io/Logger/Logger.h"
#include "Engine/Scene/DebugCamera.h"
#include "Engine/Asset/Material/Material.h"
#include "Engine/Object/Transform.h"
#include "Engine/Graphics/GPUData/TransformationMatrix.h"
#include "Engine/Graphics/GPUData/VertexData.h"
#include "Engine/Graphics/GPUData/LightsForGPU.h"
#include "Engine/Object/LightManager.h"
#include "Engine/Graphics/Pipeline/BlendMode.h"
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/Graphics/GPUData/CameraForGPU.h"
#include "Engine/Asset/Model/Node.h"
#include "Engine/Asset/Model/Mesh.h"

#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <memory>

class Model;
class InstancedModel;
class Sprite;
class ParticleSystem;
class DirectXContext;
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

	/// <summary>
	/// フレーム開始時の処理(描画開始時に行う)
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// フレーム終了時の処理(描画終了時に行う)
	/// </summary>
	void EndFrame();

private:
	DirectXContext* dxContext_ = nullptr;

	// カメラ位置(GPU転送)
	CameraForGPU* cameraData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraBuffer_;

	LightsForGPU* dummyLight_;
	Microsoft::WRL::ComPtr<ID3D12Resource> dummyLightBuffer_;
};