#pragma once


#include "../game/Project256.h"
#include <xaudio2.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;



class PlatformAudio {
	static const int BufferCount = 3;
	ComPtr<IXAudio2> mXAudio2;
	void* mMemory;
	IXAudio2MasteringVoice* mMasteringVoice;
	IXAudio2SourceVoice* mSourceVoice;
	XAUDIO2_BUFFER mBuffers[BufferCount]{};
	int mCurrentBuffer = 0;
	AudioBufferDescriptor mAudioBufferDescriptor{};

	struct SourceVoiceCallback : public IXAudio2VoiceCallback {
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
