#pragma once


#include "../game/Project256.h"
#include <xaudio2.h>
#include <wrl.h>
#include <thread>

using Microsoft::WRL::ComPtr;



class PlatformAudio {
	ComPtr<IXAudio2> mXAudio2;
	void* mMemory;
	IXAudio2MasteringVoice* mMasteringVoice;
	IXAudio2SourceVoice* mSourceVoice;
	XAUDIO2_BUFFER mBuffers[AudioBufferCount]{};
	int mCurrentBuffer = 0;
	AudioBufferDescriptor mAudioBufferDescriptor{};
	std::jthread mAudioProcessingThread;

	struct SourceVoiceCallback : public IXAudio2VoiceCallback {
		HANDLE mBufferEndEvent;
		SourceVoiceCallback();
		~SourceVoiceCallback();
		virtual void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override;
		virtual void __stdcall OnVoiceProcessingPassEnd(void) override;
		virtual void __stdcall OnStreamEnd(void) override;
		virtual void __stdcall OnBufferStart(void* pBufferContext) override;
		virtual void __stdcall OnBufferEnd(void* pBufferContext) override;
		virtual void __stdcall OnLoopEnd(void* pBufferContext) override;
		virtual void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) override;
	} mSourceVoiceCallback;

public:
	PlatformAudio(void* memory);

	void prepareNextBuffer();
};
