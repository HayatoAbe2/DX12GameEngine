#pragma once
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct IMFSourceReader;
struct IMFSample;

// デコード済みの1フレーム
struct VideoFrame {
	std::vector<uint8_t> pixels; // BGRA。上から下へ、1行あたり幅*4バイトで詰めてある
	double time = 0.0;           // 動画内の時刻(秒)
	uint32_t loopIndex = 0;      // 何周目か
};

// 動画のデコード(Media Foundation)
// デコードは専用スレッドで行う
class VideoDecoder final {
public:
	VideoDecoder() = default;
	~VideoDecoder();
	VideoDecoder(const VideoDecoder&) = delete;
	VideoDecoder& operator=(const VideoDecoder&) = delete;

	bool Open(const std::wstring& path, bool loop);
	void Close();
	// 先頭に戻す
	void Restart();

	// 再生時刻 time までに表示すべき最新フレームを取り出す。取り出せたら true
	bool PopFrame(double time, VideoFrame& out);

	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }
	bool IsEnded() const; // ループなしで、すべて取り出し終わった
	bool HasFailed() const { return failed_; }

private:
	void ThreadMain();
	bool ReadMediaType();
	bool SeekToStart();
	bool CopySample(IMFSample* sample, VideoFrame& out);
	void ReleaseReader();

	IMFSourceReader* reader_;
	std::thread thread_;
	mutable std::mutex mutex_;
	std::condition_variable cv_;
	std::deque<VideoFrame> queue_;

	bool stop_ = false;     // mutex_ で保護
	bool restart_ = false;  // mutex_ で保護
	bool ended_ = false;    // mutex_ で保護
	uint32_t loopIndex_ = 0; // mutex_ で保護
	std::atomic<bool> failed_ = false;

	bool loop_ = true;
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	int32_t stride_ = 0;    // 負なら下の行から並んでいる
	double duration_ = 0.0; // 動画の長さ(秒)

public:
	double GetDuration() const { return duration_; }
};
