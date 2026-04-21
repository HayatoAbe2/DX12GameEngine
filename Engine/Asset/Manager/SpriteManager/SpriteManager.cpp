#include "SpriteManager.h"
#include "Engine/Asset/Manager/TextureManager/TextureManager.h"
#include "Engine/Graphics/Core/DirectXContext/DirectXContext.h"
#include "Engine/Io/Logger/Logger.h"
#include "Engine/Graphics/GPUData/VertexData.h"
#include "Engine/Graphics/Core/CommandListManager/CommandListManager.h"
#include "Engine/Graphics/Core/DescriptorHeapManager/DescriptorHeapManager.h"
#include "Engine/Graphics/GPUResource/SRVManager/SRVManager.h"
#include "Engine/Graphics/GPUResource/BufferManager/BufferManager.h"
#include "Engine/Graphics/GPUResource/ConstantBufferManager/ConstantBufferManager.h"

SpriteManager::SpriteManager(DirectXContext* dxContext, Logger* logger, TextureManager* textureManager) {
	// デバイス
	device_ = dxContext->GetDeviceManager()->GetDevice().Get();
	// CommandList管理クラス
	commandListManager_ = dxContext->GetCommandListManager();
	// DescriptorHeap管理クラス
	descriptorHeapManager_ = dxContext->GetDescriptorHeapManager();
	// SRV管理クラス
	srvManager_ = dxContext->GetSRVManager();
	// バッファ管理クラス
	bufferManager_ = dxContext->GetBufferManager();
	// CB管理クラス
	cbManager_ = dxContext->GetConstantBufferManager();
	// ログ出力クラス
	logger_ = logger;
	// テクスチャ管理クラス
	textureManager_ = textureManager;
}

std::unique_ptr<Sprite> SpriteManager::Load(std::string texturePath) {
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

	sprite->SetTransformCBHandle(cbManager_->AllocateTransformCB());

	// UVTransform
	Transform uvTransform_{
		{1.0f,1.0f,0.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	// Sprite用のインデックスリソースを作成する
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = bufferManager_->CreateUploadBuffer(sizeof(uint32_t) * 6);

	// リソースの先頭のアドレスから使う
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズはindex6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_t
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;		indexDataSprite[1] = 1;		indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;		indexDataSprite[4] = 3;		indexDataSprite[5] = 2;

	sprite->SetIBV(indexBufferView);
	sprite->SetIndexResource(indexResource);

	// Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = bufferManager_->CreateUploadBuffer(sizeof(VertexData) * 4);
	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	sprite->SetVBV(vertexBufferView);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// 頂点4つ
	vertexData[0].position = { 0.0f,1.0f,0.0f,1.0f };	// 左下
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };	// 左上
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].position = { 1.0f,1.0f,0.0f,1.0f };	// 右下
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].position = { 1.0f,0.0f,0.0f,1.0f };	// 右上
	vertexData[3].texcoord = { 1.0f,0.0f };

	for (UINT i = 0; i < 4; ++i) {
		vertexData[i].normal = { 0.0f,0.0f,-1.0f };
		vertexData[i].color = { 1.0f,1.0f,1.0f,1.0f };
	}

	sprite->SetVertexResource(vertexResource);
	sprite->SetVertexData(vertexData);

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	TransformationMatrix* transformationData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationResource;
	transformationResource = bufferManager_->CreateUploadBuffer(sizeof(TransformationMatrix));

	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationData));
	// 単位行列を書き込んでおく
	transformationData->WVP = MakeIdentity4x4();
	transformationData->World = MakeIdentity4x4();
	transformationData->WorldInverseTranspose = MakeIdentity4x4();

	sprite->SetTransformData(transformationData);
	sprite->SetTransformResource(transformationResource);

	// TexturePathを設定
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->SetMtlFilePath(texturePath);
	textureManager_->CreateTextureSRV(texture);

	// Sprite用のマテリアルリソースを作る
	std::unique_ptr<Material> material = std::make_unique<Material>();
	material->Initialize(bufferManager_, true, false); // テクスチャ座標情報がなければテクスチャ不使用
	material->SetTexture(texture);
	sprite->SetMaterial(std::move(material));

	return sprite;
}