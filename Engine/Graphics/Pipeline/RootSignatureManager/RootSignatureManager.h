#pragma once
#include <wrl.h>
#include <d3d12.h>

enum class RootSignatures {
	Standard,
	Instancing,

	Particle,
	GPUParticle,

	Skybox,
	Grid,

	Fullscreen,

	SkinningCompute,

	ParticleInit,
	ParticleEmit,

	Count
};

class Logger;

/// <summary>
/// ルートシグネチャ管理
/// </summary>
class RootSignatureManager {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="logger">ログ出力</param>
	void Initialize(const Microsoft::WRL::ComPtr<ID3D12Device>& device, Logger* logger);

	// RootSignatureを取得
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature(RootSignatures type) { return rootSignatures_[(int)type].Get(); }

private:

	void CreateStandardRootSignature();
	void CreateInstancingRootSignature();
	void CreateParticleRootSignature();
	void CreateGPUParticleRootSignature();
	void CreateSkyboxRootSignature();
	void CreateGridRootSignature();
	void CreateFullscreenRootSignature();
	void CreateSkinningComputeRootSignature();
	void CreateParticleInitRootSignature();
	void CreateParticleEmitRootSignature();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignatures_[(int)RootSignatures::Count];

	// ログ出力
	Logger* logger_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
};

