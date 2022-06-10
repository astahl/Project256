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

    func gameTick() {
        let tickTimer = Chronometer()
        gameState.input.frameNumber = gameState.frameNumber
        gameState.frameNumber += 1
        let frameTime = gameState.frameTime.elapsed()
        Timings.global?.addTiming(for: .FrameToFrame, µs: frameTime.microseconds)
        gameState.upTime_microseconds += frameTime.microseconds

        gameState.input.upTime_microseconds =  gameState.upTime_microseconds
        gameState.input.elapsedTime_s = frameTime.seconds
        // TODO finalize inputs
        Timings.global?.addTiming(for: .TickSetup, µs: tickTimer.elapsed().microseconds)
        let output = doGameThings(&gameState.input, gameState.memory)
        Timings.global?.addTiming(for: .TickDo, µs: tickTimer.elapsed().microseconds)
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
        Timings.global?.addTiming(for: .TickPost, µs: tickTimer.elapsed().microseconds)
        // todo can we move update tex to its own thread and just synchronize?
        writeDrawBuffer(gameState.memory, gameState.drawBuffer.data.baseAddress!)
        Timings.global?.addTiming(for: .BufferCopy, µs: tickTimer.elapsed().microseconds)
        if gameState.frameNumber % 100 == 0 {
            print(Timings.global?.description ?? "")
            Timings.global?.clear()
        }
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
