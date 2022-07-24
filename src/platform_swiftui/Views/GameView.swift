//
//  GameView.swift
//  Project256
//
//  Created by Andreas Stahl on 13.06.22.
//

import SwiftUI

struct GameView: View {
    var state: GameState
    var drawBufferView: DrawBufferView

    init(state: GameState) {
        self.state = state
        self.drawBufferView = DrawBufferView(drawBufferSource: {
            drawBuffer in
            PlatformProfiling.withInstance {
                profiling in
                profiling.timingData.startTimer(eTimerBufferCopy)
            }
            writeDrawBuffer(state.memory, drawBuffer?.data.baseAddress!)
            PlatformProfiling.withInstance {
                profiling in
                profiling.timingData.interval(timer: eTimerBufferCopy, interval: eTimingBufferCopy)
            }
            return drawBuffer
        })
        self.drawBufferView.metalView.prefferedFrameRate = state.settings.frameTargetHz
    }

    var body: some View {
        drawBufferView
        #if os(macOS)
            .keyboardAndMouse(
                keyboard: { state.platformInput.pushKeyboardEvent($0) },
                move: { moveEvent in
                    var changedEvent = moveEvent
                    switch moveEvent {
                        case .Move(let locationInView, let relative):
                            changedEvent = .Move(locationInView: drawBufferView.pixelPosition(locationInView), relative: relative)
                        case .Drag(let locationInView, let relative, let button):
                            changedEvent = .Drag(locationInView: drawBufferView.pixelPosition(locationInView), relative: relative, button: button)
                        default: break
                    }
                    state.platformInput.pushMouseMoveEvent(changedEvent)
                },
                click: { state.platformInput.pushMouseClickEvent($0) })
        #endif
    }
}

struct GameView_Previews: PreviewProvider {
    static var previews: some View {
        GameView(state: GameState.init(settings: nil))
            .previewInterfaceOrientation(.portrait)
    }
}
