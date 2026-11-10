#include "VideoPlayer.h"
#include "Engine/Io/Logger/Logger.h"
#include <format>

VideoPlayer::VideoPlayer(DirectXContext* dxContext, Logger* logger, uint32_t textureId)
	: logger_(logger), textureId_(textureId), texture_(dxContext) {
}

bool VideoPlayer::Open(const std::string& path, bool loop) {
	Close();

	if (!decoder_.Open(logger_->ConvertString(path), loop)) {
		logger_->Log(logger_->GetStream(), std::format("[Video] Failed to open: {}\n", path));
		return false;
	}

	const uint32_t w = decoder_.GetWidth();
	const uint32_t h = decoder_.GetHeight();
	// 解像度が同じなら使い回す(貼ってあるスプライトの差し替えが要らない)
	if (!texture_.GetTexture() || texture_.GetWidth() != w || texture_.GetHeight() != h) {
		if (!texture_.Create(w, h, textureId_)) {
			logger_->Log(logger_->GetStream(), std::format("[Video] Failed to create texture: {}\n", path));
			decoder_.Close();
			return false;
		}
	}

	// 最初の1フレームが届くまで、前の映像や未初期化の内容が見えないよう黒で埋める
	std::vector<uint8_t> black(static_cast<size_t>(w) * h * 4, 0);
	texture_.Upload(black.data());

	clock_ = 0.0;
	playing_ = false;
	opened_ = true;
	return true;
}

void VideoPlayer::Close() {
	decoder_.Close();
	opened_ = false;
	playing_ = false;
}

void VideoPlayer::Restart() {
	if (!opened_) { return; }
	decoder_.Restart();
	clock_ = 0.0;
}

void VideoPlayer::Update(float deltaTime) {
	if (!opened_) { return; }
	if (playing_) { clock_ += deltaTime; }

	// 時計に合ったフレームがあれば転送する(一時停止中も、先頭の1枚は出る)
	if (decoder_.PopFrame(clock_, frame_)) {
		texture_.Upload(frame_.pixels.data());
	}
}