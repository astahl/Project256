//
//  GameState.swift
//  Project256
//
//  Created by Andreas Stahl on 28.05.22.
//

import Foundation
import SwiftUI

class GameState : ObservableObject {
    let memory = UnsafeMutableRawPointer.allocate(byteCount: MemorySize, alignment: 128)
    var input = GameInput()
    var timevalues = UnsafeMutableBufferPointer<timeval>.allocate(capacity: 2)
    var lastTimeIndex = -1
    @Published var drawBuffer = DrawBuffer(width: Int(DrawBufferWidth), height: Int(DrawBufferHeight))
}
