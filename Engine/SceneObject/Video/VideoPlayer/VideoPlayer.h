#pragma once
#include "Engine/SceneObject/Video/VideoDecoder/VideoDecoder.h"
#include "Engine/SceneObject/Video/VideoTexture/VideoTexture.h"
#include "Engine/Math/MathUtils.h"
#include <string>

class DirectXContext;
class Logger;

// 動画の再生。Update で時計を進めて、新しいフレームをテクスチャに転送する
class VideoPlayer final {
public:
	VideoPlayer(DirectXContext* dxContext, Logger* logger, uint32_t textureId);

	// 開く。失敗したら false(ログを出す)。解像度が同じなら、テクスチャを使い回す
	bool Open(const std::string& path, bool loop = true);
	void Close();

	void Play() { playing_ = true; }
	void Pause() { playing_ = false; }
	void Restart();                 // 先頭から
	void Update(float deltaTime);   // 毎フレーム呼ぶ(Update 側で。Draw 側では呼ばない)

	bool IsOpen() const { return opened_; }
	bool IsPlaying() const { return playing_; }
	bool IsFinished() const { return opened_ && decoder_.IsEnded(); }
	Vector2 GetSize() const { return { float(texture_.GetWidth()), float(texture_.GetHeight()) }; }
	const std::shared_ptr<Texture>& GetTexture() const { return texture_.GetTexture(); }

private:
	Logger* logger_ = nullptr;
	uint32_t textureId_ = 0;
	VideoDecoder decoder_;
	VideoTexture texture_;
	VideoFrame frame_;
	double clock_ = 0.0;
	bool playing_ = false;
	bool opened_ = false;
};