//
//  Project256App.swift
//  Shared
//
//  Created by Andreas Stahl on 24.05.22.
//

import SwiftUI

@main
struct Project256App: App {
    let textBuffer = UnsafeMutableRawBufferPointer.allocate(byteCount: Int(InputMaxTextLength), alignment: 64)
    @StateObject var gameState = GameState()
    @State var letterboxColor = Color.black

    func gameTick() {
        gameState.input.frameNumber = gameState.frameNumber
        gameState.frameNumber += 1
        let frameTime = gameState.frameTime.elapsed()
        gameState.input.upTime_microseconds += UInt64(frameTime.microseconds)
        gameState.input.elapsedTime_s = frameTime.seconds
        // TODO finalize inputs
        let output = doGameThings(&gameState.input, gameState.memory)

        if output.shouldQuit == eTRUE {
            exit(0)
        }
        if output.needTextInput == eTRUE {
    
        }
        #if os(macOS)
        if output.shouldHideMouse == eTRUE {
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
        gameState.input = GameInput()
    }

    var body: some Scene {
        WindowGroup {
            ZStack {
            MetalView(drawBuffer: gameState.drawBuffer)
                .letterboxColor(self.letterboxColor)
                .mouseMove {
                    relative, position in
                    inputPushMouseTrack(&gameState.input, Float(position.x), Float(position.y))
                }
                .textInput {
                    text in
                    text.utf8CString.withUnsafeBytes {
                        buffer in
                        let count = buffer.copyBytes(to: self.textBuffer) - 1 // remove \0 at end
                        inputPushUtf8Bytes(&gameState.input, self.textBuffer.baseAddress, UInt32(count))
                    }
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
