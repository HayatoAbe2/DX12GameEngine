#include "VideoDecoder.h"
#include <Windows.h>
#include <wrl.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using Microsoft::WRL::ComPtr;

namespace {
	constexpr size_t kMaxQueuedFrames = 4;  // 先読みするフレーム数
	constexpr double k100ns = 1.0e-7;       // MFの時間単位(100ナノ秒)を秒へ
}

VideoDecoder::~VideoDecoder() {
	Close();
}

bool VideoDecoder::Open(const std::wstring& path, bool loop) {
	Close();
	loop_ = loop;

	ComPtr<IMFAttributes> attributes;
	if (FAILED(MFCreateAttributes(&attributes, 1))) { return false; }
	// YUV → RGB の変換を Source Reader にやらせる
	attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

	if (FAILED(MFCreateSourceReaderFromURL(path.c_str(), attributes.Get(), &reader_))) {
		return false;
	}

	// 映像ストリームだけを使う(音声は読まない)
	reader_->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
	reader_->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

	// 出力を RGB32 (メモリ上は BGRA の並び) に指定
	ComPtr<IMFMediaType> type;
	MFCreateMediaType(&type);
	type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	type->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	if (FAILED(reader_->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, type.Get())) ||
		!ReadMediaType()) {
		ReleaseReader();
		return false;
	}

	// 動画の長さ(ループ時の時刻計算に使う)
	PROPVARIANT var;
	PropVariantInit(&var);
	if (SUCCEEDED(reader_->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))) {
		duration_ = static_cast<double>(var.uhVal.QuadPart) * k100ns;
	}
	PropVariantClear(&var);

	thread_ = std::thread(&VideoDecoder::ThreadMain, this);
	return true;
}

void VideoDecoder::Close() {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		stop_ = true;
	}
	cv_.notify_all();
	if (thread_.joinable()) { thread_.join(); }

	ReleaseReader();
	queue_.clear();
	stop_ = restart_ = ended_ = false;
	failed_ = false;
	loopIndex_ = 0;
	width_ = height_ = 0;
	duration_ = 0.0;
}

void VideoDecoder::Restart() {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		restart_ = true;
	}
	cv_.notify_all();
}

bool VideoDecoder::PopFrame(double time, VideoFrame& out) {
	bool got = false;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		// 時刻に達しているフレームは、最新の1枚だけ残して捨てる(遅れたときの追いつき)
		while (!queue_.empty()) {
			const VideoFrame& f = queue_.front();
			if (f.time + f.loopIndex * duration_ > time) { break; }
			out = std::move(queue_.front());
			queue_.pop_front();
			got = true;
		}
	}
	if (got) { cv_.notify_one(); }
	return got;
}

bool VideoDecoder::IsEnded() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return ended_ && queue_.empty();
}

bool VideoDecoder::ReadMediaType() {
	ComPtr<IMFMediaType> type;
	if (FAILED(reader_->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type))) { return false; }

	UINT32 w = 0, h = 0;
	if (FAILED(MFGetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, &w, &h))) { return false; }
	width_ = w;
	height_ = h;

	// 行のバイト数。RGB は通常、負(下から上へ並ぶ)で返ってくる
	LONG stride = static_cast<LONG>(MFGetAttributeUINT32(type.Get(), MF_MT_DEFAULT_STRIDE, 0));
	if (stride == 0) {
		MFGetStrideForBitmapInfoHeader(MFVideoFormat_RGB32.Data1, w, &stride);
	}
	stride_ = stride;
	return width_ > 0 && height_ > 0;
}

bool VideoDecoder::SeekToStart() {
	PROPVARIANT pos;
	PropVariantInit(&pos);
	pos.vt = VT_I8;
	pos.hVal.QuadPart = 0;
	HRESULT hr = reader_->SetCurrentPosition(GUID_NULL, pos);
	PropVariantClear(&pos);
	return SUCCEEDED(hr);
}

bool VideoDecoder::CopySample(IMFSample* sample, VideoFrame& out) {
	ComPtr<IMFMediaBuffer> buffer;
	if (FAILED(sample->ConvertToContiguousBuffer(&buffer))) { return false; }

	BYTE* src = nullptr;
	DWORD maxLen = 0, curLen = 0;
	if (FAILED(buffer->Lock(&src, &maxLen, &curLen))) { return false; }

	const size_t rowBytes = static_cast<size_t>(width_) * 4;
	const size_t absStride = static_cast<size_t>(std::abs(stride_));
	bool ok = curLen >= absStride * height_ && absStride >= rowBytes;
	if (ok) {
		out.pixels.resize(rowBytes * height_);
		for (uint32_t y = 0; y < height_; ++y) {
			// 負のストライド: メモリ先頭が画像の最下行
			const BYTE* row = (stride_ >= 0) ? src + y * absStride : src + (height_ - 1 - y) * absStride;
			std::memcpy(out.pixels.data() + y * rowBytes, row, rowBytes);
		}
	}
	buffer->Unlock();
	return ok;
}

void VideoDecoder::ReleaseReader() {
	if (reader_) {
		reader_->Release();
		reader_ = nullptr;
	}
}

void VideoDecoder::ThreadMain() {
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	for (;;) {
		{
			std::unique_lock<std::mutex> lock(mutex_);
			// 先読みが一杯なら待つ。終了・やり直しの要求が来たら起きる
			cv_.wait(lock, [this] {
				return stop_ || restart_ || (!ended_ && queue_.size() < kMaxQueuedFrames);
				});
			if (stop_) { break; }
			if (restart_) {
				restart_ = false;
				queue_.clear();
				ended_ = false;
				loopIndex_ = 0;
				lock.unlock();
				SeekToStart();
				continue;
			}
		}

		DWORD flags = 0;
		LONGLONG timestamp = 0;
		ComPtr<IMFSample> sample;
		HRESULT hr = reader_->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &flags, &timestamp, &sample);
		if (FAILED(hr)) {
			failed_ = true;
			std::lock_guard<std::mutex> lock(mutex_);
			ended_ = true; // 以後は再生しない(Restart で再挑戦できる)
			continue;
		}

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
			if (loop_ && SeekToStart()) {
				std::lock_guard<std::mutex> lock(mutex_);
				++loopIndex_;
			} else {
				std::lock_guard<std::mutex> lock(mutex_);
				ended_ = true;
			}
			continue;
		}
		if (!sample) { continue; } // ストリームの隙間。次を読む

		VideoFrame frame;
		if (!CopySample(sample.Get(), frame)) { continue; }
		frame.time = static_cast<double>(timestamp) * k100ns;

		std::lock_guard<std::mutex> lock(mutex_);
		if (restart_ || stop_) { continue; } // やり直し要求の前に読んだフレームは捨てる
		frame.loopIndex = loopIndex_;
		queue_.push_back(std::move(frame));
	}

	CoUninitialize();
}