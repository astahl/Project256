//
//  Project256App.swift
//  Shared
//
//  Created by Andreas Stahl on 24.05.22.
//

import SwiftUI

extension boole {
    var isTrue: Bool {
        return self == eTRUE
    }

    var isFalse: Bool {
        return self == eFALSE
    }
}


@main
struct Project256App: App {
    @StateObject var gameState = GameState()
    @State var letterboxColor = Color.black

    var profilingTimer: Timer = {
        var t = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) {
            _ in
            profiling_time_print(&GameState.timingData)
            profiling_time_clear(&GameState.timingData)
        }
        t.tolerance = 0.3
        return t
    }()
    #if os(macOS)
    func setCursorVisible(_ shouldShow: Bool)
    {
        if shouldShow && gameState.isMouseHidden {
            CGDisplayShowCursor(CGMainDisplayID())
            gameState.isMouseHidden = false
        } else if !shouldShow && !gameState.isMouseHidden {
            CGDisplayHideCursor(CGMainDisplayID())
            gameState.isMouseHidden = true
        }
    }
    #endif
    func gameTick() {
        profiling_time_interval(&GameState.timingData, eTimerFrameToFrame, eTimingFrameToFrame)
        profiling_time_set(&GameState.timingData, eTimerFrameToFrame)
        profiling_time_set(&GameState.timingData, eTimerTick)

        gameState.input.frameNumber = gameState.frameNumber
        gameState.frameNumber += 1
        let frameTime = gameState.frameTime.elapsed()
        gameState.upTime_microseconds += frameTime.microseconds

        gameState.input.upTime_microseconds =  gameState.upTime_microseconds
        gameState.input.elapsedTime_s = frameTime.seconds
        // TODO finalize inputs
        profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickSetup)

        let output = doGameThings(&gameState.input, gameState.memory)
        profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickDo)

        if output.shouldQuit.isTrue {
            exit(0)
        }
        if output.needTextInput.isTrue {
    
        }
        #if os(macOS)
        if gameState.input.mouse.endedOver.isTrue {
            setCursorVisible(output.shouldShowSystemCursor.isTrue)
        } else {
            setCursorVisible(true)
        }
        #endif
        gameState.clearInput()
        profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickPost)

        profiling_time_set(&GameState.timingData, eTimerBufferCopy)
        // todo can we move update tex to its own thread and just synchronize?
        writeDrawBuffer(gameState.memory, gameState.drawBuffer.data.baseAddress!)
        profiling_time_interval(&GameState.timingData, eTimerBufferCopy, eTimingBufferCopy)

    }

    var body: some Scene {
        WindowGroup {
            ZStack {
            MetalView(drawBuffer: gameState.drawBuffer)
                .letterboxColor(self.letterboxColor)
                .mouseMove {
                    relative, position in
                    gameState.addInputMouseMovement(relative: relative, position: position)
                }
                .mouseClick {
                    button, click, position in

                    switch (button, click)
                    {
                    case (.Left, let upOrDown) where upOrDown == .Up || upOrDown == .Down:
                        gameState.input.mouse.buttonLeft.transitionCount += 1
                        gameState.input.mouse.buttonLeft.endedDown = upOrDown == .Down ? eTRUE : eFALSE
                    case (.Right, let upOrDown) where upOrDown == .Up || upOrDown == .Down:
                        gameState.input.mouse.buttonRight.transitionCount += 1
                        gameState.input.mouse.buttonRight.endedDown = upOrDown == .Down ? eTRUE : eFALSE
                    case (.Other, let upOrDown) where upOrDown == .Up || upOrDown == .Down:
                        gameState.input.mouse.buttonMiddle.transitionCount += 1
                        gameState.input.mouse.buttonMiddle.endedDown = upOrDown == .Down ? eTRUE : eFALSE
                    default:
                        break;
                    }

                }
                .textInput {
                    text in
                    gameState.addInputText(text: text)
                }
                .beforeDraw {
                    self.gameTick()
                }

            }
        }
        #if os(macOS)
        Settings {
            VStack {
                ColorPicker("Letterbox", selection: $letterboxColor)
            }.padding()
        }
        #endif
    }
}
