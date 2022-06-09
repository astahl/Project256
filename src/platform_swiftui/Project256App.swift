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

    func gameTick() {
        gameState.input.frameNumber = gameState.frameNumber
        gameState.frameNumber += 1
        let frameTime = gameState.frameTime.elapsed()
        gameState.upTime_microseconds += frameTime.microseconds

        gameState.input.upTime_microseconds =  gameState.upTime_microseconds
        gameState.input.elapsedTime_s = frameTime.seconds
        // TODO finalize inputs
        let output = doGameThings(&gameState.input, gameState.memory)

        if output.shouldQuit.isTrue {
            exit(0)
        }
        if output.needTextInput.isTrue {
    
        }
        #if os(macOS)
        if output.shouldHideMouse.isTrue {
            if !gameState.isMouseHidden {
                CGDisplayHideCursor(CGMainDisplayID())
                gameState.isMouseHidden = true
            }
        } else {
            if gameState.isMouseHidden {
                CGDisplayShowCursor(CGMainDisplayID())
                gameState.isMouseHidden = false
            }
        }
        #endif
        // todo can we move update tex to its own thread and just synchronize?
        writeDrawBuffer(gameState.memory, gameState.drawBuffer.data.baseAddress!)
        gameState.clearInput()
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
                .textInput {
                    text in
                    gameState.addInputText(text: text)
                }
                .beforeDraw(self.gameTick)

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
