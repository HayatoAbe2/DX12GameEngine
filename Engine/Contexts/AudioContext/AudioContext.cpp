#include "AudioContext.h"

AudioContext::AudioContext(AudioSystem* audio) {
	audio_ = audio;
}

void AudioContext::SoundLoad(const wchar_t* filename) {
	audio_->SoundLoad(filename);
}

void AudioContext::SoundPlay(const wchar_t* filename, bool isLoop, float volume) {
	audio_->SoundPlay(filename, isLoop, volume);
}

void AudioContext::SoundUnload(const wchar_t* filename) {
	audio_->SoundUnload(filename);
}