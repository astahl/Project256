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


class GameState : ObservableObject {
    let settings: GameSettings
    let platformInput: PlatformInput
    let platformAudio: PlatformAudio

    static var timingData: ProfilingTime? = {
        var value = ProfilingTime()
        value.getPlatformTimeMicroseconds = timestamp
        return value
    }()
    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    let frameTime = Chronometer()
    var isMouseHidden = false

    var drawBuffer = DrawBuffer()
    let platformCallbacks = PlatformCallbacks(readFile: loadDataDEBUG(filenamePtr:destination:bufferSize:), readImage: loadImageDEBUG(filenamePtr:destination:width:height:))

    init(settings: GameSettings?) {
        self.settings = settings ?? GameSettings()
        self.platformInput = PlatformInput(settings: self.settings)
        self.platformAudio = PlatformAudio(memory: self.memory)
    }

    func tick() {
        GameState.timingData?.interval(timer: eTimerTickToTick, interval: eTimingTickToTick)
        GameState.timingData?.startTimer(eTimerTickToTick)
        GameState.timingData?.startTimer(eTimerTick)

        self.platformInput.updateGameInput(gameInput: &self.input, frameTime: self.frameTime.elapsed())
       
        GameState.timingData?.interval(timer: eTimerTick, interval: eTimingTickSetup)

        let output = doGameThings(&self.input, self.memory, self.platformCallbacks)
        GameState.timingData?.interval(timer: eTimerTick, interval: eTimingTickDo)

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
        GameState.timingData?.interval(timer: eTimerTick, interval: eTimingTickPost)
    }
}
