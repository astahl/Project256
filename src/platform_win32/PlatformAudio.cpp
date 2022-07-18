#include "PlatformAudio.h"
#include <cassert>

void ExitOnFail(HRESULT hr) {
	if (FAILED(hr)) {
		assert(false);
		exit(hr);
	}
}

PlatformAudio::PlatformAudio(void* memory) :mMemory{ memory } {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	ExitOnFail(hr);

	hr = XAudio2Create(mXAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	ExitOnFail(hr);

	hr = mXAudio2->CreateMasteringVoice(&mMasteringVoice);
	ExitOnFail(hr);

	WAVEFORMATEX waveformat{
		.wFormatTag = WAVE_FORMAT_PCM,
		.nChannels = AudioChannelsPerFrame,
		.nSamplesPerSec = AudioFramesPerSecond,
		.nAvgBytesPerSec = AudioFramesPerSecond * AudioBitsPerSample / 8 * AudioChannelsPerFrame,
		.nBlockAlign = AudioChannelsPerFrame * AudioBitsPerSample / 8,
		.wBitsPerSample = AudioBitsPerSample,
	};
	mXAudio2->CreateSourceVoice(&mSourceVoice, &waveformat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &mSourceVoiceCallback, nullptr, nullptr);
	
	XAUDIO2_VOICE_DETAILS details;
	mSourceVoice->GetVoiceDetails(&details);

	mAudioBufferDescriptor.channelsPerFrame = 2;
	mAudioBufferDescriptor.framesPerBuffer = AudioFramesPerBuffer;
	mAudioBufferDescriptor.sampleRate = details.InputSampleRate;
	mAudioBufferDescriptor.sampleTime = 0;
	mAudioBufferDescriptor.timestamp = 0;

	const int bufferSize = AudioFramesPerBuffer * AudioBitsPerSample / 8 * AudioChannelsPerFrame;
	BYTE* bufferBytes = reinterpret_cast<BYTE*>(VirtualAlloc(0, bufferSize * AudioBufferCount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	ptrdiff_t index = 0;
	for (auto& buffer : mBuffers) {
		buffer.AudioBytes = bufferSize;
		buffer.pAudioData = bufferBytes;
		buffer.pContext = reinterpret_cast<void*>(index++);
		mSourceVoice->SubmitSourceBuffer(&buffer);
		bufferBytes += bufferSize;
	}

	mAudioProcessingThread = std::jthread{ [&](std::stop_token stop, HANDLE waitable) {
		DWORD waitResult = WAIT_TIMEOUT;
		while (true) {
			do {
				if (stop.stop_requested()) {
					return;
				}
				waitResult = WaitForSingleObjectEx(waitable, 200, FALSE);
			} while (waitResult == WAIT_TIMEOUT);

			if (waitResult == WAIT_OBJECT_0) {
				prepareNextBuffer();
			}
		}
	}, mSourceVoiceCallback.mBufferEndEvent };

	mSourceVoice->Start();
}

void PlatformAudio::prepareNextBuffer() {
	XAUDIO2_BUFFER& buf = mBuffers[mCurrentBuffer];
	void* data = const_cast<void*>(reinterpret_cast<const void*>(buf.pAudioData));
	writeAudioBuffer(this->mMemory, data, mAudioBufferDescriptor);
	mSourceVoice->SubmitSourceBuffer(&buf);
	//std::stringstream x{};
	//x << "Submit: " << mCurrentBuffer << "\n";
	//OutputDebugStringA(x.str().c_str());
	mCurrentBuffer = (mCurrentBuffer + 1) % AudioBufferCount;
	mAudioBufferDescriptor.sampleTime += mAudioBufferDescriptor.framesPerBuffer;
}

PlatformAudio::SourceVoiceCallback::SourceVoiceCallback()
	: mBufferEndEvent{CreateEvent(NULL, FALSE, FALSE, NULL)}
{
}

PlatformAudio::SourceVoiceCallback::~SourceVoiceCallback()
{
	CloseHandle(mBufferEndEvent);
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnVoiceProcessingPassStart(UINT32)
{
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnVoiceProcessingPassEnd()
{
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnStreamEnd()
{
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnBufferStart(void* index)
{
	//auto bufferIndex = reinterpret_cast<ptrdiff_t>(index);
	//std::stringstream x{};
	//x << "Start: " << bufferIndex << "\n";
	//OutputDebugStringA(x.str().c_str());
	index = index;
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnBufferEnd(void* index)
{
	//auto bufferIndex = reinterpret_cast<ptrdiff_t>(index);
	//std::stringstream x{};
	//x << "End: " << bufferIndex << "\n";
	//OutputDebugStringA(x.str().c_str());
	index = index;
	SetEvent(mBufferEndEvent);
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnLoopEnd(void*)
{
}

void __stdcall PlatformAudio::SourceVoiceCallback::OnVoiceError(void*, HRESULT)
{
}
