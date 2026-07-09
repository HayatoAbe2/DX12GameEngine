#pragma once
#include <wrl.h>
#include <d3d12.h>

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

	void CreateStandardRootSignature();
	void CreateInstancingRootSignature();
	void CreateParticleRootSignature();
	void CreateSkyboxRootSignature();
	void CreateGridRootSignature();
	void CreateFullscreenRootSignature();
	void CreateSkinningComputeRootSignature();

	/// <summary>
	/// RootSignatureを取得
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetStandardRootSignature() { return standardRootSignature_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetInstancingRootSignature() { return instancingRootSignature_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetParticleRootSignature() { return particleRootSignature_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetSkyboxRootSignature() { return skyboxRootSignature_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetGridRootSignature() { return gridRootSignature_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetFullscreenRootSignature() { return fullscreenRootSignature_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetSkinningComputeRootSignature() { return skinningComputeRootSignature_.Get(); }

private:

	Microsoft::WRL::ComPtr<ID3D12RootSignature> standardRootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> instancingRootSignature_ = nullptr; // インスタンス描画用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> particleRootSignature_ = nullptr; // パーティクル用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skyboxRootSignature_ = nullptr; // skybox
	Microsoft::WRL::ComPtr<ID3D12RootSignature> gridRootSignature_ = nullptr; // grid
	Microsoft::WRL::ComPtr<ID3D12RootSignature> fullscreenRootSignature_ = nullptr; // オフスクリーンコピー用
	Microsoft::WRL::ComPtr<ID3D12RootSignature> skinningComputeRootSignature_ = nullptr; // CSスキニング

	// ログ出力
	Logger* logger_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
};

