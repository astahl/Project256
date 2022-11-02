//
//  GameState.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI

#if os(macOS)
func setCursorVisible(_ shouldShow: Bool, currentlyHidden: Bool) -> Bool
{
    if shouldShow && currentlyHidden {
        CGDisplayShowCursor(CGMainDisplayID())
        return false
    } else if !shouldShow && !currentlyHidden {
        CGDisplayHideCursor(CGMainDisplayID())
        return true
    }
    return currentlyHidden
}
#endif


class GameState {
    let settings: GameSettings
    let platformInput: PlatformInput
    let platformAudio: PlatformAudio

    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    let frameTime = Chronometer()
    var isMouseHidden = false

    var drawBuffer = DrawBuffer()
    let platformCallbacks = PlatformCallbacks(readFile: loadDataDEBUG(filenamePtr:destination:bufferSize:), readImage: loadImageDEBUG(filenamePtr:destination:width:height:), log: printDEBUG(utf8StringPtr:))

    init(settings: GameSettings?) {
        memory.initializeMemory(as: UInt8.self, repeating: UInt8.zero, count: MemorySize)
        self.settings = settings ?? GameSettings()
        self.platformInput = PlatformInput(settings: self.settings, input: &self.input)
        self.platformAudio = PlatformAudio(memory: self.memory)
        self.platformAudio.start()
    }

    func tick() {
        PlatformProfiling.withInstance {
            profiling in
            profiling.timingData.interval(timer: eTimerTickToTick, interval: eTimingTickToTick)
            profiling.timingData.startTimer(eTimerTickToTick)
            profiling.timingData.startTimer(eTimerTick)
        }

        self.platformInput.updateGameInput(frameTime: self.frameTime.elapsed())
       
        PlatformProfiling.withInstance {
            profiling in
            profiling.timingData.interval(timer: eTimerTick, interval: eTimingTickSetup)
        }

        let output = doGameThings(&self.input, self.memory, self.platformCallbacks)
        PlatformProfiling.withInstance {
            profiling in
            profiling.timingData.interval(timer: eTimerTick, interval: eTimingTickDo)
        }

        if output.shouldQuit {
            exit(0)
        }
        if output.needTextInput {
            // todo show keyboard on iOS
        }
        #if os(macOS)
//        if self.input.mouse.endedOver {
//            self.isMouseHidden = setCursorVisible(output.shouldShowSystemCursor, currentlyHidden: self.isMouseHidden)
//        } else {
//            self.isMouseHidden = setCursorVisible(true, currentlyHidden: self.isMouseHidden)
//        }
        #endif
        cleanInput(&input)
        PlatformProfiling.withInstance {
            profiling in
            profiling.timingData.interval(timer: eTimerTick, interval: eTimingTickPost)
        }
    }
}
