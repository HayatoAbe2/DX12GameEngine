#pragma once
#include "Engine/Scene/DebugCamera/DebugCamera.h"
#include "Engine/Graphics/GPUData/LightsForGPU.h"
#include "Engine/SceneObject/LightManager/LightManager.h"
#include "Engine/Graphics/GPUData/CameraForGPU.h"
#include "Engine/SceneObject/Model/Node.h"
#include "Engine/SceneObject/Model/Mesh.h"
#include "Engine/SceneObject/Primitive/Primitive.h"

#include "Engine/Graphics/Renderer/PostEffectType.h"
#include "Engine/Graphics/GPUData/TransformationMatrix.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"

#include <wrl.h>
#include <d3d12.h>
#include "Engine/Editor/Scene/SceneEditor/GizmoCtx.h"
#include "Engine/Graphics/SkinningSystem/SkinningSystem.h"
#include "Engine/Graphics/Renderer/ParticleCompute/ParticleCompute.h"

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
	void DrawModel(Model* model, LightManager* lightManager, BlendMode blendMode);

	/// <summary>
	/// インスタンスモデル描画
	/// </summary>
	/// <param name="model">複数インスタンスを持つモデル</param>
	/// <param name="blendMode">ブレンドモード</param>
	void DrawModelInstance(InstancedModel* model, LightManager* lightManager, BlendMode blendMode);

	void DrawParticles(ParticleSystem* particleSys, BlendMode blendMode);
	void DrawGPUParticle(ParticleSystem* particleSys, BlendMode blendMode);
	void DrawPrimitive(Primitive* primitive, BlendMode blendMode);
	void DrawSprite(Sprite* sprite, BlendMode blendMode);

	// ノードごとに描画
	void DrawNode(Model* model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parent);

	// メッシュを描画
	void DrawMesh(Model* model, const MeshData& mesh, const MeshRuntime& meshRuntim);

	// インスタンシング描画版
	void DrawNodeInstance(InstancedModel* model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld);
	void DrawMeshInstance(InstancedModel* model, const MeshData& mesh);

	// Skybox描画
	void DrawSkybox(Texture* texture);

	// ポストエフェクト設定
	void AddPostEffect(PostEffectType type) { dxContext_->AddPostEffect(type); }
	void SetDissolveMask(D3D12_GPU_DESCRIPTOR_HANDLE handle);

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

	void BeginPass(RenderPass pass, bool clear) { dxContext_->BeginPass(pass, clear); }

#ifdef USE_IMGUI
	bool GetSceneWindowHovered() { return dxContext_->GetSceneWindowHovered(); }
	void SetGizmoCtx(const GizmoCtx& ctx) { dxContext_->SetGizmoCtx(ctx); }
#endif
private:
	void InitializePlane();
	void InitializeRing();
	void InitializeCylinder();
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
	ShapeData cylinder_;
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
	Microsoft::WRL::ComPtr<ID3D12Resource> perViewResource_;
	PerView* perViewData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource_;
	PerFrame* perFrameData_ = nullptr;

	std::unique_ptr<SkinningSystem> skinningSystem_;
	std::unique_ptr<ParticleCompute> particleCompute_;

};