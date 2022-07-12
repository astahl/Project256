//
//  PlatformAudio.swift
//  Project256
//
//  Created by Andreas Stahl on 10.07.22.
//

import Foundation
import AudioToolbox

class PlatformAudio {

    var audioQueueRef: AudioQueueRef?
    let framesPerBuffer: UInt32 = AudioFramesPerBuffer
    let memory: UnsafeMutableRawPointer

    var thread: Thread?
    let condition = NSCondition()
    var availableBuffers: [AudioQueueBufferRef] = []
    var queueLock = NSLock()

    init(memory: UnsafeMutableRawPointer) {
        self.memory = memory


        let bytesPerSample: UInt32 = AudioBitsPerSample / 8
        let channelsPerFrame: UInt32 = AudioChannelsPerFrame
        let bytesPerFrame: UInt32 = bytesPerSample * channelsPerFrame
        let bufferSize: UInt32 = bytesPerFrame * framesPerBuffer

        var audioStreamDescription = AudioStreamBasicDescription(
            mSampleRate: Double(AudioFramesPerSecond),
            mFormatID: kAudioFormatLinearPCM,
            mFormatFlags: kLinearPCMFormatFlagIsSignedInteger,
            mBytesPerPacket: bytesPerFrame,
            mFramesPerPacket: 1,
            mBytesPerFrame: bytesPerFrame,
            mChannelsPerFrame: channelsPerFrame,
            mBitsPerChannel: AudioBitsPerSample,
            mReserved: 0)

        var status = AudioQueueNewOutputWithDispatchQueue(&self.audioQueueRef, &audioStreamDescription, 0, DispatchQueue.global(qos: .userInteractive)) { audioQueue, buffer in
            GameState.timingData?.interval(timer: eTimerAudioBufferToAudioBuffer, interval: eTimingAudioBufferToAudioBuffer)
            GameState.timingData?.startTimer(eTimerAudioBufferToAudioBuffer)

            self.queueLock.lock()
            defer {
                self.queueLock.unlock()
                self.condition.signal()
            }
            self.availableBuffers.append(buffer)
        }
        if status != noErr {
            exit(status)
        }

        guard let audioQueue = self.audioQueueRef else {
            exit(status)
        }

        for _ in 0..<AudioBufferCount {
            var buffer: AudioQueueBufferRef?
            status = AudioQueueAllocateBuffer(audioQueue, bufferSize, &buffer)
            if status != noErr {
                exit(status)
            }
            self.availableBuffers.append(buffer!)
        }

        status = AudioQueueStart(audioQueue, nil)
        if status != noErr {
            exit(status)
        }
    }

    func start() {
        self.thread = .init(block: {
            var buffer: AudioQueueBufferRef
            while !self.thread!.isCancelled {
                self.condition.lock()
                while (self.availableBuffers.isEmpty) {
                    self.condition.wait()
                }

                self.queueLock.lock()
                buffer = self.availableBuffers.removeFirst()
                self.queueLock.unlock()

                if let aq = self.audioQueueRef {
                    self.fillAndEnqueueIn(aq, bufferRef: buffer)
                }
                self.condition.unlock()
            }
        })

        self.thread?.start()
    }

    func stop() {
        self.thread?.cancel()
    }

    func fillAndEnqueueIn(_ queue: AudioQueueRef, bufferRef: AudioQueueBufferRef) {
        GameState.timingData?.startTimer(eTimerFillAudioBuffer)
        let capacity = bufferRef.pointee.mAudioDataBytesCapacity

        var timestamp: AudioTimeStamp = .init()
        AudioQueueDeviceGetCurrentTime(queue, &timestamp)

        let descriptor = AudioBufferDescriptor(timestamp: timestamp.mHostTime, sampleTime: timestamp.mSampleTime, sampleRate: Double(AudioFramesPerSecond), framesPerBuffer: framesPerBuffer, channelsPerFrame: 2)
        writeAudioBuffer(self.memory, bufferRef.pointee.mAudioData, descriptor)

        bufferRef.pointee.mAudioDataByteSize = capacity
        let status = AudioQueueEnqueueBuffer(queue, bufferRef, 0, nil)
        if status != noErr {
            exit(status)
        }
        GameState.timingData?.interval(timer: eTimerFillAudioBuffer, interval: eTimingFillAudioBuffer)
    }
}
