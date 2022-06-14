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
    }

    var body: some View {
        drawBufferView
        #if os(macOS)
            .keyboardAndMouse(keyboard: { event in
                switch event {
                case .Down(_, let characters?):
                    gameState.addInputText(characters)
                case .Down(_,_):
                    break
                case .Up(_,_):
                    break
                }
            }, move: { move in
                switch move {
                case .Move(let locationInView, let relative):
                    let position = drawBufferView.pixelPosition(locationInView)
                    gameState.addInputMouseMovement(relative: relative, position: position)
                case .Drag(let locationInView, let relative, _):
                    let position = drawBufferView.pixelPosition(locationInView)
                    gameState.addInputMouseMovement(relative: relative, position: position)
                case .Scroll(_):
                    break
                }
            }, click: { click in
                switch click {
                case .Down(.Left, _):
                    gameState.input.mouse.buttonLeft.transitionCount += 1
                    gameState.input.mouse.buttonLeft.endedDown = true
                case .Down(.Right, _):
                    gameState.input.mouse.buttonRight.transitionCount += 1
                    gameState.input.mouse.buttonRight.endedDown = true
                case .Down(.Other, _):
                    gameState.input.mouse.buttonMiddle.transitionCount += 1
                    gameState.input.mouse.buttonMiddle.endedDown = true
                case .Up(.Left, _):
                    gameState.input.mouse.buttonLeft.transitionCount += 1
                    gameState.input.mouse.buttonLeft.endedDown = false
                case .Up(.Right, _):
                    gameState.input.mouse.buttonRight.transitionCount += 1
                    gameState.input.mouse.buttonRight.endedDown = false
                case .Up(.Other, _):
                    gameState.input.mouse.buttonMiddle.transitionCount += 1
                    gameState.input.mouse.buttonMiddle.endedDown = false

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
