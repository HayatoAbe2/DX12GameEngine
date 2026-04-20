#pragma once
#include "Engine/Io/AudioSystem/AudioSystem.h"

// 音声関連関数の窓口
class AudioContext {
public:
	AudioContext(AudioSystem* audio);

	void SoundLoad(const wchar_t* filename);

	// 再生(先に読み込みが必要)
	void SoundPlay(const wchar_t* filename, bool isLoop, float volume = 1.0f);

	// 削除(任意。呼ばなくても終了時に消える)
	void SoundUnload(const wchar_t* filename);

private:
	AudioSystem* audio_ = nullptr;
};

