//
//  GameView.swift
//  Project256
//
//  Created by Andreas Stahl on 13.06.22.
//

import SwiftUI

struct GameView: View {
    var state: GameState
    var settings: GameSettings
    var drawBufferView: DrawBufferView

    init(state: GameState, settings: GameSettings) {
        self.state = state
        self.settings = settings
        self.drawBufferView = DrawBufferView(drawBufferSource: {
            drawBuffer in
            GameState.timingData?.startTimer(eTimerBufferCopy)
            writeDrawBuffer(state.memory, drawBuffer?.data.baseAddress!)
            GameState.timingData?.interval(timer: eTimerBufferCopy, interval: eTimingBufferCopy)
            return drawBuffer
        })
        self.drawBufferView.metalView.prefferedFrameRate = settings.frameTargetHz
    }

    var body: some View {
        drawBufferView
        #if os(macOS)
            .keyboardAndMouse(keyboard: {
                switch $0 {
                case .Down(_, let characters?, let modifiers):
                    state.addInputText(characters)
                    print(modifiers)
                default:
                    break
                }
            }, move: {
                switch $0 {
                case .Move(let locationInView, let relative):
                    let position = drawBufferView.pixelPosition(locationInView)
                    state.addInputMouseMovement(relative: relative, position: position)
                case .Drag(let locationInView, let relative, _):
                    let position = drawBufferView.pixelPosition(locationInView)
                    state.addInputMouseMovement(relative: relative, position: position)
                case .Scroll(_):
                    break
                }
            }, click: {
                switch $0 {
                case .Down(.Left, _):
                    state.input.mouse.buttonLeft.press()
                case .Down(.Right, _):
                    state.input.mouse.buttonRight.press()
                case .Down(.Other, _):
                    state.input.mouse.buttonMiddle.press()
                case .Up(.Left, _):
                    state.input.mouse.buttonLeft.release()
                case .Up(.Right, _):
                    state.input.mouse.buttonRight.release()
                case .Up(.Other, _):
                    state.input.mouse.buttonMiddle.release()

                }
            })
        #endif
    }
}

struct GameView_Previews: PreviewProvider {
    static var previews: some View {
        GameView(state: GameState.init(), settings: GameSettings())
            .previewInterfaceOrientation(.portrait)
    }
}
