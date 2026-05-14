#include <Windows.h>
#include "Renderer.h"
#include "Engine/Graphics/Core/CommandListManager/CommandListManager.h"
#include "Engine/Graphics/Pipeline/PipelineStateManager/PipelineStateManager.h"
#include "Engine/Graphics/Pipeline/RootSignatureManager/RootSignatureManager.h"
#include "Engine/Graphics/Core/DeviceManager/DeviceManager.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"
#include "Engine/Graphics/GPUResource/BufferManager/BufferManager.h"
#include "Engine/Asset/Model/Model.h"
#include "Engine/Asset/Model/InstancedModel.h"
#include "Engine/Asset/Sprite/Sprite.h"
#include "Engine/Object/Particle/ParticleSystem/ParticleSystem.h"
#include "Engine/Scene/Camera/Camera.h"

void Renderer::Initialize(DirectXContext* dxContext) {
	dxContext_ = dxContext;

	// デバイス
	auto device = dxContext_->GetDeviceManager()->GetDevice();

	// CB管理クラス
	cbManager_ = dxContext->GetConstantBufferManager();

	// カメラバッファ作成
	cameraBuffer_ = dxContext_->GetBufferManager()->CreateUploadBuffer(sizeof(CameraForGPU));
	cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	// ライト非使用時のダミー
	dummyLightBuffer_ = dxContext_->GetBufferManager()->CreateUploadBuffer(sizeof(LightsForGPU));
	dummyLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dummyLight_));

	InitializeSkybox();
}

void Renderer::UpdateSpriteTransform(Sprite* sprite) {
	Transform transform{};
	Vector2 size = sprite->GetSize();
	Vector2 pos = sprite->GetPosition();
	float rot = sprite->GetRotation();
	Vector2 windowSize = { float(dxContext_->GetWindowWidth()),float(dxContext_->GetWindowHeight()) };

	transform.scale = { size.x, size.y, 1.0f };
	transform.translate = { pos.x,pos.y, 0.0f };
	transform.rotate = { 0.0f,0.0f,rot };
	// トランスフォーム
	TransformationMatrix data;
	data.World = MakeAffineMatrix(transform);
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, windowSize.x, windowSize.y, 0.0f, 100.0f);
	data.WVP = Multiply(data.World, projectionMatrix);
	data.WorldInverseTranspose = Transpose(Inverse(data.World));

	auto cbAddress = dxContext_->GetConstantBufferManager()->UploadTransform(data); // gpu送信
	// トランスフォームCBV
	dxContext_->GetCommandListManager()->GetCommandList()->SetGraphicsRootConstantBufferView(1, cbAddress);
}

void Renderer::DrawModel(Model* model, Camera* camera, LightManager* lightManager, int blendMode) {
	// GPUに渡すデータの更新
	cameraData_->position = camera->transform_.translate;
	if (lightManager) { lightManager->Update(); }

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetStandardPSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetStandardRootSignature().Get();

	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// カメラ
	cmdList->SetGraphicsRootConstantBufferView(4, cameraBuffer_->GetGPUVirtualAddress());
	// ライト
	cmdList->SetGraphicsRootConstantBufferView(5, lightManager->GetLightResource()->GetGPUVirtualAddress());

	DrawNode(model, camera, cmdList, model->GetRootNode(), MakeIdentity4x4());
}

void Renderer::DrawModelInstance(InstancedModel* model, Camera* camera, LightManager* lightManager, int blendMode) {
	// GPUに渡すデータの更新
	cameraData_->position = camera->transform_.translate;
	if (lightManager) { lightManager->Update(); }

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetInstancingPSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetInstancingRootSignature().Get();

	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// wvp用のCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(1, model->GetInstanceCBV());
	// インスタンス用SRVの設定
	cmdList->SetGraphicsRootDescriptorTable(4, model->GetInstanceSRVHandle());
	// カメラ
	cmdList->SetGraphicsRootConstantBufferView(5, cameraBuffer_->GetGPUVirtualAddress());
	if (lightManager) {
		// ライト
		cmdList->SetGraphicsRootConstantBufferView(6, lightManager->GetLightResource()->GetGPUVirtualAddress());
	} else {
		cmdList->SetGraphicsRootConstantBufferView(6, dummyLightBuffer_->GetGPUVirtualAddress());
	}

	DrawNodeInstance(model, camera, cmdList, model->GetRootNode(), MakeIdentity4x4());
}

void Renderer::DrawParticles(ParticleSystem* particleSys, Camera* camera, int blendMode) {
	particleSys->PreDraw(camera);

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetParticlePSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetParticleRootSignature().Get();

	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 各メッシュを描画
	for (auto& mesh : particleSys->GetInstancedModel()->GetData()->meshes) {
		for (const auto& subMesh : mesh->GetPrimitives()) {

			Material* material = particleSys->GetInstancedModel()->GetMaterial(subMesh.materialIndex_);
			// マテリアル更新
			material->UpdateGPU();

			// マテリアルCBufferの場所を設定
			cmdList->SetGraphicsRootConstantBufferView(0, material->GetCBV()->GetGPUVirtualAddress());
			// モデル描画
			cmdList->IASetVertexBuffers(0, 1, &subMesh.vertexBufferView_);	// VBVを設定
			// wvp用のCBufferの場所を設定
			cmdList->SetGraphicsRootConstantBufferView(1, particleSys->GetInstancedModel()->GetInstanceCBV());
			// SRVの設定
			cmdList->SetGraphicsRootDescriptorTable(2, material->GetTextureSRVHandle());
			// インスタンス用SRVの設定
			cmdList->SetGraphicsRootDescriptorTable(3, particleSys->GetInstancedModel()->GetInstanceSRVHandle());
			// ドローコール
			cmdList->DrawInstanced(UINT(subMesh.vertices_.size()), particleSys->GetInstancedModel()->GetNumInstance(), 0, 0);
		}
	}
}

void Renderer::DrawSprite(Sprite* sprite, int blendMode) {
	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetStandardPSO(blendMode);
	auto rootSig = dxContext_->GetRootSignatureManager()->GetStandardRootSignature().Get();

	sprite->UpdateMaterial();
	// PSO設定
	cmdList->SetPipelineState(pso);
	// RootSignatureを設定
	cmdList->SetGraphicsRootSignature(rootSig);
	// トポロジを三角形に設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UpdateSpriteTransform(sprite);
	// マテリアルCBufferの場所を設定
	cmdList->SetGraphicsRootConstantBufferView(0, sprite->GetMaterialCBV());
	// Spriteの描画。変更が必要なものだけ変更する
	cmdList->IASetIndexBuffer(&sprite->GetIBV());	// IBVを設定
	cmdList->IASetVertexBuffers(0, 1, &sprite->GetVBV());	// VBVを設定
	// SRVの設定
	cmdList->SetGraphicsRootDescriptorTable(2, sprite->GetTextureSRVHandle());
	// ドローコール
	cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Renderer::DrawNode(Model* model, Camera* camera, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld) {
	Matrix4x4 modelWorld = MakeAffineMatrix(model->GetTransform());
	Matrix4x4 nodeWorld = parentWorld * node->localMatrix;

	// トランスフォーム更新
	TransformationMatrix data;
	data.World = modelWorld * nodeWorld;
	data.WVP = data.World
		* camera->viewMatrix_
		* camera->projectionMatrix_;
	data.WorldInverseTranspose = Transpose(Inverse(data.World));
	auto cbAddress = dxContext_->GetConstantBufferManager()->UploadTransform(data); // gpu送信
	cmdList->SetGraphicsRootConstantBufferView(1, cbAddress);

	// メッシュを描画
	for (uint32_t meshIndex : node->meshIndices) {
		DrawMesh(model, model->GetData()->meshes[meshIndex].get());
	}

	// 子ノード
	for (auto& child : node->children) {
		DrawNode(model, camera, cmdList, child.get(), nodeWorld);
	}
}

void Renderer::DrawMesh(Model* model, Mesh* mesh) {
	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();

	for (const auto& subMesh : mesh->GetPrimitives()) {
		Material* material = model->GetMaterial(subMesh.materialIndex_);

		// マテリアル更新
		material->UpdateGPU();

		// マテリアルCBufferの場所を設定
		cmdList->SetGraphicsRootConstantBufferView(0, material->GetCBV()->GetGPUVirtualAddress());
		// メッシュVBV
		cmdList->IASetVertexBuffers(0, 1, &subMesh.vertexBufferView_);	// VBVを設定
		// IBV
		cmdList->IASetIndexBuffer(&subMesh.ibv_);
		// SRVの設定
		cmdList->SetGraphicsRootDescriptorTable(2, material->GetTextureSRVHandle());
		cmdList->SetGraphicsRootDescriptorTable(3, material->GetEnvironmentTextureSRVHandle());
		// ドローコール
		cmdList->DrawIndexedInstanced(UINT(subMesh.indices_.size()), 1, 0, 0, 0);
	}
}

void Renderer::DrawNodeInstance(InstancedModel* model, Camera* camera, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList, ModelNode* node, const Matrix4x4& parentWorld) {
	Matrix4x4 nodeWorld = parentWorld * node->localMatrix;

	// トランスフォーム更新
	std::vector<Vector4> colors;
	colors.resize(model->GetNumInstance());
	for (auto& color : colors) {
		color = { 1,1,1,1 };
	}
	model->UpdateInstanceTransformWithNode(camera, nodeWorld, colors);

	// メッシュを描画
	for (uint32_t meshIndex : node->meshIndices) {
		DrawMeshInstance(model, model->GetData()->meshes[meshIndex].get());
	}

	// 子ノード
	for (auto& child : node->children) {
		DrawNodeInstance(model, camera, cmdList, child.get(), nodeWorld);
	}
}

void Renderer::DrawMeshInstance(InstancedModel* model, Mesh* mesh) {
	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();

	// 各メッシュを描画
	for (const auto& subMesh : mesh->GetPrimitives()) {
		Material* material = model->GetMaterial(subMesh.materialIndex_);

		// マテリアル更新
		material->UpdateGPU();

		// マテリアルCBufferの場所を設定
		cmdList->SetGraphicsRootConstantBufferView(0, material->GetCBV()->GetGPUVirtualAddress());
		// モデル描画
		cmdList->IASetVertexBuffers(0, 1, &subMesh.vertexBufferView_);	// VBVを設定
		// SRVの設定
		cmdList->SetGraphicsRootDescriptorTable(2, material->GetTextureSRVHandle());
		cmdList->SetGraphicsRootDescriptorTable(3, material->GetEnvironmentTextureSRVHandle());
		// ドローコール
		cmdList->DrawInstanced(UINT(subMesh.vertices_.size()), model->GetNumInstance(), 0, 0);
	}

}

void Renderer::InitializeSkybox() {
	auto bufferManager = dxContext_->GetBufferManager();
	UINT sizeInBytes = sizeof(VertexData) * 24;

	// 頂点リソース
	skybox.vertexResource = bufferManager->CreateUploadBuffer(sizeInBytes);
	// VBV
	skybox.vbv.BufferLocation = skybox.vertexResource->GetGPUVirtualAddress();
	skybox.vbv.SizeInBytes = sizeInBytes;
	skybox.vbv.StrideInBytes = sizeof(VertexData);

	// 頂点を設定
	VertexData* vertices = nullptr;
	skybox.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertices));
	// 右(0,1,2),(2,1,3)
	vertices[0] = { {1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[1] = { {1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[2] = { {1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[3] = { {1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 左(4,5,6),(6,5,7)
	vertices[4] = { {-1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[5] = { {-1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[6] = { {-1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[7] = { {-1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 前(8,9,10),(10,9,11)
	vertices[8] = { {-1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[9] = { {1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[10] = { {-1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[11] = { {1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 後(12,13,14),(14,13,15)
	vertices[12] = { {1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[13] = { {-1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[14] = { {1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[15] = { {-1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 上(16,17,18),(18,17,19)
	vertices[16] = { {-1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[17] = { {1.0f, 1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[18] = { {-1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[19] = { {1.0f, 1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	// 下(20,21,22),(22,21,23)
	vertices[20] = { {-1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[21] = { {1.0f, -1.0f, 1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[22] = { {-1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };
	vertices[23] = { {1.0f, -1.0f, -1.0f, 1.0f}, {}, {}, {1.0f, 1.0f, 1.0f, 1.0f} };

	// indexリソース
	skybox.indexResource = bufferManager->CreateUploadBuffer(sizeof(uint32_t) * 36);
	// IBV
	skybox.ibv.BufferLocation = skybox.indexResource->GetGPUVirtualAddress();
	skybox.ibv.SizeInBytes = sizeof(uint32_t) * 36;
	skybox.ibv.Format = DXGI_FORMAT_R32_UINT;
	// データ
	uint32_t* indexData = nullptr;
	skybox.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;	indexData[1] = 1;	indexData[2] = 2; indexData[3] = 1;	indexData[4] = 3;	indexData[5] = 2;
	indexData[6] = 4;	indexData[7] = 5;	indexData[8] = 6; indexData[9] = 6;	indexData[10] = 5;	indexData[11] = 7;
	indexData[12] = 8;	indexData[13] = 9;	indexData[14] = 10; indexData[15] = 10;	indexData[16] = 9;	indexData[17] = 11;
	indexData[18] = 12;	indexData[19] = 13;	indexData[20] = 14; indexData[21] = 14;	indexData[22] = 13;	indexData[23] = 15;
	indexData[24] = 16;	indexData[25] = 17;	indexData[26] = 18; indexData[27] = 18;	indexData[28] = 17;	indexData[29] = 19;
	indexData[30] = 20;	indexData[31] = 21;	indexData[32] = 22; indexData[33] = 22;	indexData[34] = 21;	indexData[35] = 23;

	// Material
	skybox.materialResource =
		bufferManager->CreateUploadBuffer(sizeof(Material));

	skybox.materialResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&skybox.materialData));

	MaterialData data;
	data.color = { 1,1,1,1 };
	data.uvTransform = MakeIdentity4x4();
	data.enableLighting = false;
	data.useTexture = true;
	skybox.materialData->SetData(data);

	// Transform
	skybox.transformResource =
		bufferManager->CreateUploadBuffer(
			sizeof(TransformationMatrix));

	skybox.transformResource->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&skybox.transformData));

	skybox.transformData->WVP = MakeIdentity4x4();
	skybox.transformData->World = MakeAffineMatrix(Transform({ {1000,1000,1000},{},{} }));
	skybox.transformData->WorldInverseTranspose = MakeIdentity4x4();
}

void Renderer::DrawSkybox(Texture* texture, Camera* camera) {
	skybox.transformData->WVP = skybox.transformData->World * camera->viewMatrix_ * camera->projectionMatrix_;

	auto cmdList = dxContext_->GetCommandListManager()->GetCommandList();
	auto pso = dxContext_->GetPipelineStateManager()->GetSkyboxPSO(int(BlendMode::Normal));
	auto rootSig = dxContext_->GetRootSignatureManager()->GetSkyboxRootSignature().Get();

	cmdList->SetPipelineState(pso);
	cmdList->SetGraphicsRootSignature(rootSig);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
	cmdList->IASetVertexBuffers(0, 1, &skybox.vbv);
	cmdList->IASetIndexBuffer(&skybox.ibv);
	cmdList->SetGraphicsRootConstantBufferView(0, skybox.materialResource->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, skybox.transformResource->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(2, texture->GetSRVHandle());
	cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void Renderer::BeginFrame() {
	dxContext_->BeginFrame();
}

void Renderer::EndFrame() {
	dxContext_->EndFrame();
}