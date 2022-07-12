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
    let framesPerBuffer: UInt32 = 256
    let memory: UnsafeMutableRawPointer

    init(memory: UnsafeMutableRawPointer) {
        self.memory = memory
        let bytesPerSample: UInt32 = 2
        let channelsPerFrame: UInt32 = 2
        let bytesPerFrame: UInt32 = bytesPerSample * channelsPerFrame
        let bufferSize: UInt32 = bytesPerFrame * framesPerBuffer

        var audioStreamDescription = AudioStreamBasicDescription(
            mSampleRate: 48000.0,
            mFormatID: kAudioFormatLinearPCM,
            mFormatFlags: kLinearPCMFormatFlagIsSignedInteger,
            mBytesPerPacket: bytesPerFrame,
            mFramesPerPacket: 1,
            mBytesPerFrame: bytesPerFrame,
            mChannelsPerFrame: channelsPerFrame,
            mBitsPerChannel: bytesPerSample * 8,
            mReserved: 0)

        var status = AudioQueueNewOutputWithDispatchQueue(&self.audioQueueRef, &audioStreamDescription, 0, DispatchQueue.global(qos: .userInteractive)) { audioQueue, buffer in
            self.fillAndEnqueueIn(audioQueue, bufferRef: buffer)
        }
        if status != noErr {
            exit(status)
        }

        guard let audioQueue = self.audioQueueRef else {
            exit(status)
        }

        for _ in 0..<5 {
            var buffer: AudioQueueBufferRef?
            status = AudioQueueAllocateBuffer(audioQueue, bufferSize, &buffer)
            if status != noErr {
                exit(status)
            }
            fillAndEnqueueIn(audioQueue, bufferRef: buffer!)
        }

        status = AudioQueueStart(audioQueue, nil)
        if status != noErr {
            exit(status)
        }
    }

    func fillAndEnqueueIn(_ queue: AudioQueueRef, bufferRef: AudioQueueBufferRef) {
        let capacity = bufferRef.pointee.mAudioDataBytesCapacity

        var timestamp: AudioTimeStamp = .init()
        AudioQueueDeviceGetCurrentTime(queue, &timestamp)

        let descriptor = AudioBufferDescriptor(timestamp: timestamp.mHostTime, sampleTime: timestamp.mSampleTime, sampleRate: 48000.0, framesPerBuffer: framesPerBuffer, channelsPerFrame: 2)
        writeAudioBuffer(self.memory, bufferRef.pointee.mAudioData, descriptor)

        bufferRef.pointee.mAudioDataByteSize = capacity
        let status = AudioQueueEnqueueBuffer(queue, bufferRef, 0, nil)
        if status != noErr {
            exit(status)
        }
    }
}
