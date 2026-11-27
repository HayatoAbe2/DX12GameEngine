#pragma once
#include "Engine/Graphics/Pipeline/BlendMode.h"
#include "Engine/Graphics/Pipeline/PipelineStateManager/PipelineStateManager.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <d3d12.h>
#include <wrl.h>

class Logger;
class ShaderCompiler;

/// <summary>
/// 登録したシェーダーを指すハンドル
/// </summary>
struct ShaderHandle {
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFF;

	uint32_t index = kInvalidIndex;

	// 登録済みのシェーダーを指しているか(既定値は無効。無効なら標準のシェーダーで描画される)
	bool IsValid() const { return index != kInvalidIndex; }
};

/// <summary>
/// シェーダー登録時の設定
/// </summary>
struct ShaderDesc {
	std::wstring vs;	// 頂点シェーダーのhlslファイルパス
	std::wstring ps;	// ピクセルシェーダーのhlslファイルパス

	// PSOの設定
	MainPSOType base = MainPSOType::Standard;
};

/// <summary>
/// ゲームコードから追加したシェーダーの管理
/// </summary>
class ShaderLibrary {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="logger">ログ出力</param>
	/// <param name="shaderCompiler">シェーダーコンパイル</param>
	/// <param name="pipelineStateManager">PSO管理(初期化済みのもの)</param>
	void Initialize(Logger* logger, ShaderCompiler* shaderCompiler, PipelineStateManager* pipelineStateManager);

	/// <summary>
	/// シェーダーを登録(コンパイルとPSO作成を行うため、初期化時に1回だけ行う)
	/// </summary>
	/// <param name="name">登録名。登録済みの名前なら再コンパイルせずに登録済みのハンドルを返す</param>
	/// <param name="desc">シェーダーの設定</param>
	/// <returns>描画時に渡すハンドル</returns>
	ShaderHandle Register(const std::string& name, const ShaderDesc& desc);

	/// <summary>
	/// 登録名からハンドルを取得(未登録なら無効なハンドル)
	/// </summary>
	ShaderHandle Find(const std::string& name) const;

	// PSOを取得
	ID3D12PipelineState* GetPSO(ShaderHandle handle, BlendMode blendMode) const;

	// RootSignatureを取得
	ID3D12RootSignature* GetRootSignature(ShaderHandle handle) const;

	// 流用元のモデル用の種類を取得
	MainPSOType GetBase(ShaderHandle handle) const;

private:
	struct ShaderData {
		MainPSOType base = MainPSOType::Standard;
		std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, size_t(BlendMode::Count)> pso;
	};

	std::vector<ShaderData> shaders_;
	std::unordered_map<std::string, uint32_t> nameToIndex_;

	//
	// 参照
	//

	// ログ出力
	Logger* logger_ = nullptr;

	// シェーダーコンパイル
	ShaderCompiler* shaderCompiler_ = nullptr;

	// PSO管理
	PipelineStateManager* pipelineStateManager_ = nullptr;
};

