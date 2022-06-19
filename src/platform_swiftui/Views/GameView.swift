//
//  GameView.swift
//  Project256
//
//  Created by Andreas Stahl on 13.06.22.
//

import SwiftUI

struct GameView: View {
    var gameState: GameState

    var drawBufferView: DrawBufferView

    init(gameState: GameState) {
        self.gameState = gameState
        self.drawBufferView = DrawBufferView(drawBufferSource: {
            drawBuffer in
            profiling_time_set(&GameState.timingData, eTimerBufferCopy)
            writeDrawBuffer(gameState.memory, drawBuffer?.data.baseAddress!)
            profiling_time_interval(&GameState.timingData, eTimerBufferCopy, eTimingBufferCopy)
            return drawBuffer
        })
        self.drawBufferView.metalView.prefferedFrameRate = self.gameState.frameTargetHz
    }

    var body: some View {
        drawBufferView
        #if os(macOS)
            .keyboardAndMouse(keyboard: {
                switch $0 {
                case .Down(_, let characters?):
                    gameState.addInputText(characters)
                default:
                    break
                }
            }, move: {
                switch $0 {
                case .Move(let locationInView, let relative):
                    let position = drawBufferView.pixelPosition(locationInView)
                    gameState.addInputMouseMovement(relative: relative, position: position)
                case .Drag(let locationInView, let relative, _):
                    let position = drawBufferView.pixelPosition(locationInView)
                    gameState.addInputMouseMovement(relative: relative, position: position)
                case .Scroll(_):
                    break
                }
            }, click: {
                switch $0 {
                case .Down(.Left, _):
                    gameState.input.mouse.buttonLeft.press()
                case .Down(.Right, _):
                    gameState.input.mouse.buttonRight.press()
                case .Down(.Other, _):
                    gameState.input.mouse.buttonMiddle.press()
                case .Up(.Left, _):
                    gameState.input.mouse.buttonLeft.release()
                case .Up(.Right, _):
                    gameState.input.mouse.buttonRight.release()
                case .Up(.Other, _):
                    gameState.input.mouse.buttonMiddle.release()

                }
            })
        #endif
    }
}

struct GameView_Previews: PreviewProvider {
    static var previews: some View {
        GameView(gameState: GameState.init())
            .previewInterfaceOrientation(.portrait)
    }
}
