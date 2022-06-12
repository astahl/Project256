//
//  Project256App.swift
//  Shared
//
//  Created by Andreas Stahl on 24.05.22.
//

import SwiftUI
import Combine


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


func tick(gameState: GameState) {
    profiling_time_interval(&GameState.timingData, eTimerTickToTick, eTimingTickToTick)
    profiling_time_set(&GameState.timingData, eTimerTickToTick)
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

    if output.shouldQuit {
        exit(0)
    }
    if output.needTextInput {

    }
    #if os(macOS)
    if gameState.input.mouse.endedOver {
        gameState.isMouseHidden =
        setCursorVisible(output.shouldShowSystemCursor, currentlyHidden: gameState.isMouseHidden)
    } else {
        gameState.isMouseHidden = setCursorVisible(true, currentlyHidden: gameState.isMouseHidden)
    }
    #endif
    gameState.clearInput()
    profiling_time_interval(&GameState.timingData, eTimerTick, eTimingTickPost)
}


@main
struct Project256App: App {
    class AppSubscriptions {
        var profiling: AnyCancellable? = nil
        var highfrequency: AnyCancellable? = nil
    }

    @State var letterboxColor = Color.mint

    var gameState: GameState

    var profilingBuffer = UnsafeMutableBufferPointer<CChar>.allocate(capacity: 1000)
    var subscriptions: AppSubscriptions

    init() {
        gameState = GameState()
        subscriptions = AppSubscriptions()
    }

    func doTick(_ _: Date) {
        tick(gameState: gameState)
    }

    var body: some Scene {
        WindowGroup {
            ZStack {
            MetalView()
                .mouseMove(gameState.addInputMouseMovement(relative:position:))
                .mouseClick {
                    button, click, position in

                    switch (button, click)
                    {
                    case (.Left, let upOrDown) where upOrDown == .Up || upOrDown == .Down:
                        gameState.input.mouse.buttonLeft.transitionCount += 1
                        gameState.input.mouse.buttonLeft.endedDown = upOrDown == .Down
                    case (.Right, let upOrDown) where upOrDown == .Up || upOrDown == .Down:
                        gameState.input.mouse.buttonRight.transitionCount += 1
                        gameState.input.mouse.buttonRight.endedDown = upOrDown == .Down
                    case (.Other, let upOrDown) where upOrDown == .Up || upOrDown == .Down:
                        gameState.input.mouse.buttonMiddle.transitionCount += 1
                        gameState.input.mouse.buttonMiddle.endedDown = upOrDown == .Down
                    default:
                        break;
                    }

                }
                .textInput (gameState.addInputText)
                .beforeDraw {
                    drawBuffer in
                    profiling_time_set(&GameState.timingData, eTimerBufferCopy)
                    writeDrawBuffer(gameState.memory, drawBuffer.data.baseAddress!)
                    profiling_time_interval(&GameState.timingData, eTimerBufferCopy, eTimingBufferCopy)
                }
                .onAppear {
                    self.subscriptions.highfrequency = Timer.publish(every: 0.01, on: .main, in: .default)
                        .autoconnect()
                        .sink(receiveValue: self.doTick)
                    self.subscriptions.profiling = Timer.publish(every: 1.0, on: .main, in: .default)
                        .autoconnect()
                        .sink {
                            date in
                            let length = profiling_time_print(&GameState.timingData, profilingBuffer.baseAddress!, Int32(profilingBuffer.count))
                            let profiling = String.init(bytesNoCopy: profilingBuffer.baseAddress!, length: Int(length), encoding: .ascii, freeWhenDone: false)

                            print(date)
                            print(profiling!)
                            profiling_time_clear(&GameState.timingData)
                        }
                }
                .onDisappear {
                    self.subscriptions.profiling?.cancel()
                    self.subscriptions.highfrequency?.cancel()
                }
                .background(.linearGradient(.init(colors: [Color.cyan, Color.purple]), startPoint: .topLeading, endPoint: .bottomTrailing))
                .overlay(Ellipse().foregroundColor(.gray).opacity(0.3).blur(radius: 100))
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
